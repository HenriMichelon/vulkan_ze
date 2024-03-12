#include "z0/vulkan/default_renderer.hpp"
#include "z0/log.hpp"

namespace z0 {

    DefaultRenderer::DefaultRenderer(VulkanDevice &dev,
                                     const std::string& sDir) :
         VulkanRenderer{dev, sDir}
     {}

    DefaultRenderer::~DefaultRenderer() {
        vkDeviceWaitIdle(device);
    }

    void DefaultRenderer::loadScene(const std::shared_ptr<Node>& root) {
        rootNode = root;
        loadNode(rootNode);
        createImagesIndex(rootNode);
        createResources();
    }

    void DefaultRenderer::loadNode(std::shared_ptr<Node>& node) {
        if (currentCamera == nullptr) {
            if (auto camera = dynamic_cast<Camera*>(node.get())) {
                currentCamera = camera;
                log("Using camera", currentCamera->toString());
            }
        }
        if (directionalLight == nullptr) {
            if (auto light = dynamic_cast<DirectionalLight*>(node.get())) {
                directionalLight = light;
                log("Using directional light", directionalLight->toString());
            }
        }
        if (environement == nullptr) {
            if (auto env = dynamic_cast<Environment*>(node.get())) {
                environement = env;
                log("Using environment", environement->toString());
            }
        }
        if (auto omniLight = dynamic_cast<OmniLight *>(node.get())) {
            omniLights.push_back(omniLight);
        }
        createImagesList(node);
        for(auto& child: node->getChildren()) {
            loadNode(child);
        }
    }

    void DefaultRenderer::createImagesList(std::shared_ptr<Node>& node) {
        if (auto meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            meshes.push_back(meshInstance);
            for(const auto& material : meshInstance->getMesh()->_getMaterials()) {
                if (auto standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                    if (standardMaterial->albedoTexture != nullptr) {
                        images.insert(standardMaterial->albedoTexture->getImage()._getImage());
                    }
                    if (standardMaterial->specularTexture != nullptr) {
                        images.insert(standardMaterial->specularTexture->getImage()._getImage());
                    }
                }
            }
        }
    }
    void DefaultRenderer::createImagesIndex(std::shared_ptr<Node>& node) {
        if (auto meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            for(const auto& material : meshInstance->getMesh()->_getMaterials()) {
                if (auto standardMaterial = dynamic_cast<StandardMaterial*>(material.get())) {
                    if (standardMaterial->albedoTexture != nullptr) {
                        auto& image = standardMaterial->albedoTexture->getImage();
                        auto index = std::distance(std::begin(images), images.find(image._getImage()));
                        imagesIndices[image.getId()] = static_cast<int32_t>(index);
                    }
                    if (standardMaterial->specularTexture != nullptr) {
                        auto& image = standardMaterial->specularTexture->getImage();
                        auto index = std::distance(std::begin(images), images.find(image._getImage()));
                        imagesIndices[image.getId()] = static_cast<int32_t>(index);
                    }
                }
            }
        }
        for(auto& child: node->getChildren()) {
            createImagesIndex(child);
        }
    }

    void DefaultRenderer::loadShaders() {
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void DefaultRenderer::update(float delta) {
        if (meshes.empty() || currentCamera == nullptr) return;

        GobalUniformBufferObject globalUbo{
            .projection = currentCamera->getProjection(),
            .view = currentCamera->getView(),
            .cameraPosition = currentCamera->getPosition(),
        };
        if (directionalLight != nullptr) {
            globalUbo.directionalLight = {
                .direction = directionalLight->getDirection(),
                .color = directionalLight->getColorAndIntensity(),
                .specular = directionalLight->getSpecularIntensity(),
            };
            globalUbo.haveDirectionalLight = true;
        }
        if (environement != nullptr) {
            globalUbo.ambient = environement->getAmbientColorAndIntensity();
        }
        globalUbo.pointLightsCount = omniLights.size();
        writeUniformBuffer(globalBuffers, &globalUbo);

        auto pointLightsArray =  std::make_unique<PointLightUniform[]>(globalUbo.pointLightsCount);
        for(int i=0; i < globalUbo.pointLightsCount; i++) {
            pointLightsArray[i].position = omniLights[i]->getPosition();
            pointLightsArray[i].color = omniLights[i]->getColorAndIntensity();
            pointLightsArray[i].specular = omniLights[i]->getSpecularIntensity();
            pointLightsArray[i].constant = omniLights[i]->getAttenuation();
            pointLightsArray[i].linear = omniLights[i]->getLinear();
            pointLightsArray[i].quadratic = omniLights[i]->getQuadratic();
            if (auto spot = dynamic_cast<SpotLight*>(omniLights[i])) {
                pointLightsArray[i].isSpot = true;
                pointLightsArray[i].direction = spot->getDirection();
                pointLightsArray[i].cutOff = spot->getCutOff();
                pointLightsArray[i].outerCutOff =spot->getOuterCutOff();
            }
        }
        writeUniformBuffer(pointLightBuffers, pointLightsArray.get());

        uint32_t modelIndex = 0;
        uint32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            ModelUniformBufferObject modelUbo {
                .matrix = meshInstance->getGlobalTransform(),
                .normalMatrix = meshInstance->getGlobalNormalTransform(),
            };
            writeUniformBuffer(modelsBuffers, &modelUbo, modelIndex);
            if (meshInstance->getMesh()->isValid()) {
                for (const auto &surface: meshInstance->getMesh()->getSurfaces()) {
                    SurfaceUniformBufferObject surfaceUbo { };
                    if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                        surfaceUbo.albedoColor = standardMaterial->albedoColor.color;
                        if (standardMaterial->albedoTexture != nullptr) {
                            surfaceUbo.diffuseIndex = imagesIndices[standardMaterial->albedoTexture->getImage().getId()];
                        }
                        if (standardMaterial->specularTexture != nullptr) {
                            surfaceUbo.specularIndex = imagesIndices[standardMaterial->specularTexture->getImage().getId()];
                        }
                    }
                    writeUniformBuffer(surfacesBuffers, &surfaceUbo, surfaceIndex);
                    surfaceIndex += 1;
                }
            }
            modelIndex += 1;
        }
    }

    void DefaultRenderer::recordCommands(VkCommandBuffer commandBuffer) {
        if (meshes.empty() || currentCamera == nullptr) return;
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        bindShader(commandBuffer, *vertShader);
        bindShader(commandBuffer, *fragShader);

        uint32_t modelIndex = 0;
        uint32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            auto mesh = meshInstance->getMesh();
            if (mesh->isValid()) {
                for (const auto& surface: mesh->getSurfaces()) {
                    if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                        vkCmdSetCullMode(commandBuffer,
                                         standardMaterial->cullMode == CULLMODE_DISABLED ? VK_CULL_MODE_NONE :
                                         standardMaterial->cullMode == CULLMODE_BACK ? VK_CULL_MODE_BACK_BIT
                                                                             : VK_CULL_MODE_FRONT_BIT);
                    } else {
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
                    }
                    std::array<uint32_t, 4> offsets = {
                        0, // globalBuffers
                        static_cast<uint32_t>(modelsBuffers[currentFrame]->getAlignmentSize() * modelIndex),
                        static_cast<uint32_t>(surfacesBuffers[currentFrame]->getAlignmentSize() * surfaceIndex),
                        0, // pointLightBuffers
                    };
                    bindDescriptorSets(commandBuffer, offsets.size(), offsets.data());
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    surfaceIndex += 1;
                }
            }
            modelIndex += 1;
        }
    }

    void DefaultRenderer::createDescriptorSetLayout() {
        if (meshes.empty() || currentCamera == nullptr) return;
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT) // textures
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // surfaces UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // pointlightarray UBO
                .build();

        // Global UBO
        createUniformBuffers(globalBuffers, sizeof(GobalUniformBufferObject));

        // Models UBO
        VkDeviceSize modelBufferSize = sizeof(ModelUniformBufferObject);
        createUniformBuffers(modelsBuffers, modelBufferSize, meshes.size());

        // Surface materials UBO
        VkDeviceSize surfaceBufferSize = sizeof(SurfaceUniformBufferObject);
        uint32_t surfaceCount = 0;
        for (const auto& meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                surfaceCount += meshInstance->getMesh()->getSurfaces().size();
            }
        }
        createUniformBuffers(surfacesBuffers, surfaceBufferSize, surfaceCount);

        // PointLight array UBO
        VkDeviceSize pointLightBufferSize = sizeof(PointLightUniform) * (omniLights.size()+ (omniLights.empty() ? 1 : 0));
        createUniformBuffers(pointLightBuffers, pointLightBufferSize);

        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0, // global UBO
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, // textures
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       VK_SHADER_STAGE_FRAGMENT_BIT,
                       images.size())
            .addBinding(2, // model UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(3, // surfaces UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(4, // PointLight array UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        for (int i = 0; i < surfacesDescriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GobalUniformBufferObject));
            auto modelBufferInfo = modelsBuffers[i]->descriptorInfo(modelBufferSize);
            auto surfaceBufferInfo = surfacesBuffers[i]->descriptorInfo(surfaceBufferSize);
            auto pointLightBufferInfo = pointLightBuffers[i]->descriptorInfo(pointLightBufferSize);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            for(const auto& image : images) {
                imagesInfo.push_back(image->imageInfo());
            }
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeImage(1, imagesInfo.data())
                .writeBuffer(2, &modelBufferInfo)
                .writeBuffer(3, &surfaceBufferInfo)
                .writeBuffer(4, &pointLightBufferInfo)
                .build(surfacesDescriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

}