// Using one descriptor per scene with offsets
// https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
#include "z0/vulkan/renderers/scene_renderer.hpp"
#include "z0/log.hpp"

namespace z0 {

    SceneRenderer::SceneRenderer(VulkanDevice &dev,
                                 std::string sDir) :
            BaseRenderer{dev, sDir}
     {
         createImagesResources();
     }

    void SceneRenderer::cleanup() {
        if (shadowMap != nullptr) {
            shadowMapRenderer->cleanup();
            shadowMapRenderer.reset();
            shadowMap.reset();
        }
        //depthPrepassRenderer->cleanup();
        depthBuffer.reset();
        images.clear();
        pointLightBuffers.clear();
        surfacesBuffers.clear();
        modelsBuffers.clear();
        BaseRenderer::cleanup();
    }

    void SceneRenderer::loadScene(std::shared_ptr<Node>& rootNode) {
        loadNode(rootNode);
        createImagesIndex(rootNode);
        createResources();
        if (shadowMap != nullptr) {
            shadowMapRenderer = std::make_shared<ShadowMapRenderer>(vulkanDevice, shaderDirectory);
            shadowMapRenderer->loadScene(shadowMap, meshes);
            vulkanDevice.registerRenderer(shadowMapRenderer);
        }
        //depthPrepassRenderer->loadScene(depthBuffer, currentCamera, meshes);
        //vulkanDevice.registerRenderer(depthPrepassRenderer);
    }

    void SceneRenderer::loadNode(std::shared_ptr<Node>& parent) {
        if (currentCamera == nullptr) {
            if (auto camera = dynamic_cast<Camera*>(parent.get())) {
                currentCamera = camera;
                log("Using camera", currentCamera->toString());
            }
        }
        if (directionalLight == nullptr) {
            if (auto light = dynamic_cast<DirectionalLight*>(parent.get())) {
                directionalLight = light;
                log("Using directional light", directionalLight->toString());
            }
        }
        if (environement == nullptr) {
            if (auto env = dynamic_cast<Environment*>(parent.get())) {
                environement = env;
                log("Using environment", environement->toString());
            }
        }
        if (auto omniLight = dynamic_cast<OmniLight *>(parent.get())) {
            omniLights.push_back(omniLight);
            if (auto spotLight = dynamic_cast<SpotLight *>(parent.get())) {
                if (shadowMap == nullptr) shadowMap = std::make_shared<ShadowMap>(vulkanDevice, spotLight);
            }
        }
        createImagesList(parent);
        for(auto& child: parent->getChildren()) {
            loadNode(child);
        }
    }

    void SceneRenderer::createImagesList(std::shared_ptr<Node>& node) {
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
    void SceneRenderer::createImagesIndex(std::shared_ptr<Node>& node) {
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

    void SceneRenderer::loadShaders() {
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SceneRenderer::update(uint32_t currentFrame) {
        if (meshes.empty() || currentCamera == nullptr) return;
        GobalUniformBufferObject globalUbo{
            .projection = currentCamera->getProjection(),
            .view = currentCamera->getView(),
            .cameraPosition = currentCamera->getPosition(),
            .haveShadowMap = false, // shadowMap != nullptr,
        };
        if (shadowMap != nullptr) {
            glm::mat4 lightProjection = glm::perspective(shadowMap->getLight()->getFov(), 1.0f, 0.1f, 100.0f);
            glm::mat4 lightView = glm::lookAt(shadowMap->getLight()->getPosition(), glm::vec3(0.0f), glm::vec3(0, 1, 0));
            globalUbo.lightSpace = lightProjection * lightView;
            globalUbo.lightPos = shadowMap->getLight()->getPosition();
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
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);

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
        writeUniformBuffer(pointLightBuffers, currentFrame, pointLightsArray.get());

        uint32_t modelIndex = 0;
        uint32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            ModelUniformBufferObject modelUbo {
                .matrix = meshInstance->getGlobalTransform(),
                .normalMatrix = meshInstance->getGlobalNormalTransform(),
            };
            writeUniformBuffer(modelsBuffers, currentFrame, &modelUbo, modelIndex);
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
                    writeUniformBuffer(surfacesBuffers, currentFrame, &surfaceUbo, surfaceIndex);
                    surfaceIndex += 1;
                }
            }
            modelIndex += 1;
        }
    }

    void SceneRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (meshes.empty() || currentCamera == nullptr) return;
        bindShader(commandBuffer, *vertShader);
        bindShader(commandBuffer, *fragShader);
        setViewport(commandBuffer, vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, vulkanDevice.getSamples());
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);

        setViewport(commandBuffer, vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height);

        // quad renderer
/*
        vkCmdSetVertexInputEXT(commandBuffer, 0, nullptr, 0, nullptr);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        std::array<uint32_t, 4> offsets = {
                0, // globalBuffers
                0,
                0,
                0, // pointLightBuffers
        };
        bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        return;
*/
        std::vector<VkVertexInputBindingDescription2EXT> vertexBinding = VulkanModel::getBindingDescription();
        std::vector<VkVertexInputAttributeDescription2EXT> vertexAttribute = VulkanModel::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());

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
                    bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    surfaceIndex += 1;
                }
            }
            modelIndex += 1;
        }

    }

    void SceneRenderer::createDescriptorSetLayout() {
        if (meshes.empty() || currentCamera == nullptr) return;
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT) // textures
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // surfaces UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // pointlightarray UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT) // shadow map
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
            .addBinding(5, // shadow map
                   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                   VK_SHADER_STAGE_FRAGMENT_BIT)
           .build();

        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GobalUniformBufferObject));
            auto modelBufferInfo = modelsBuffers[i]->descriptorInfo(modelBufferSize);
            auto surfaceBufferInfo = surfacesBuffers[i]->descriptorInfo(surfaceBufferSize);
            auto pointLightBufferInfo = pointLightBuffers[i]->descriptorInfo(pointLightBufferSize);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            for(const auto& image : images) {
                imagesInfo.push_back(image->imageInfo());
            }
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeImage(1, imagesInfo.data())
                .writeBuffer(2, &modelBufferInfo)
                .writeBuffer(3, &surfaceBufferInfo)
                .writeBuffer(4, &pointLightBufferInfo);
             if (shadowMap != nullptr) {
                VkDescriptorImageInfo imageInfo {
                        .sampler = shadowMap->getSampler(),
                        .imageView = shadowMap->getImageView(),
                        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                };
                //VkDescriptorImageInfo imageInfo = imagesInfo[1];
                writer.writeImage(5, &imageInfo);
            } else {
                 VkDescriptorImageInfo imageInfo = imagesInfo[0]; // find a better solution (blank image ?)
                 writer.writeImage(5, &imageInfo);
             }
            if (!writer.build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void SceneRenderer::recreateImagesResources() {
        cleanupImagesResources();
        createImagesResources();
    }

    void SceneRenderer::createImagesResources() {
        // Create Color Resources (where we draw)
        // https://vulkan-tutorial.com/Multisampling#page_Setting-up-a-render-target
        VkFormat colorFormat = vulkanDevice.getSwapChainImageFormat();
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height,
                                 1, vulkanDevice.getSamples(), colorFormat,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 colorImage, colorImageMemory);
        colorImageView = vulkanDevice.createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

        colorImageResolve.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        colorImageResolve.srcOffset = {0, 0, 0};
        colorImageResolve.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        colorImageResolve.dstOffset = {0, 0, 0};
        colorImageResolve.extent = {
                vulkanDevice.getSwapChainExtent().width,
                vulkanDevice.getSwapChainExtent().height,
                1};

        colorImageBlit.srcOffsets[0] = {0, 0, 0 };
        colorImageBlit.srcOffsets[1] = {
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().width),
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().height), 1 };
        colorImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.srcSubresource.mipLevel = 0;
        colorImageBlit.srcSubresource.baseArrayLayer = 0;
        colorImageBlit.srcSubresource.layerCount = 1;
        colorImageBlit.dstOffsets[0] = {0, 0, 0 };
        colorImageBlit.dstOffsets[1] = {
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().width),
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().height), 1 };
        colorImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.dstSubresource.mipLevel = 0;
        colorImageBlit.dstSubresource.baseArrayLayer = 0;
        colorImageBlit.dstSubresource.layerCount = 1;

        depthBuffer = std::make_shared<DepthBuffer>(vulkanDevice);
        /*if (depthPrepassRenderer == nullptr) {
            depthPrepassRenderer = std::make_shared<DepthPrepassRenderer>(vulkanDevice, shaderDirectory);
        }*/
    }

    void SceneRenderer::cleanupImagesResources() {
        vkDestroyImageView(device, colorImageView, nullptr);
        vkDestroyImage(device, colorImage, nullptr);
        vkFreeMemory(device, colorImageMemory, nullptr);
    }

    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void SceneRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           depthBuffer->getImage(),
                                           depthBuffer->getFormat(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           colorImage,
                                           vulkanDevice.getSwapChainImageFormat(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        // Color attachement : where the rendering is done (multisampled memory image)
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorImageView,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = depthBuffer->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = depthClearValue,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, vulkanDevice.getSwapChainExtent()},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void SceneRenderer::endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(
                commandBuffer,
                swapChainImage,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Since we render in a memory image we need to manually present the image in the swap chain
        if (vulkanDevice.getSamples() == VK_SAMPLE_COUNT_1_BIT) {
            // Blit image to swap chain if MSAA is disabled
            vkCmdBlitImage(commandBuffer,
                           colorImage,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           swapChainImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &colorImageBlit,
                           VK_FILTER_LINEAR );
        } else {
            // Resolve multisample image to a non-multisample swap chain image if MSAA is enabled
            vkCmdResolveImage(commandBuffer,
                              colorImage,
                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              swapChainImage,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              1,
                              &colorImageResolve);
        }

        vulkanDevice.transitionImageLayout(
                commandBuffer,
                swapChainImage,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }



}