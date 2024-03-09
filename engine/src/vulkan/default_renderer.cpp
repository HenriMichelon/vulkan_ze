#include "z0/vulkan/default_renderer.hpp"
#include "z0/log.hpp"
#include "z0/nodes/camera.hpp"

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
        createMeshIndices(rootNode);
        createResources(); //sizeof(PushConstants));
    }

    void DefaultRenderer::createMeshIndices(std::shared_ptr<Node>& parent) {
        createMeshIndex(parent);
        for(auto& node: parent->getChildren()) {
            createMeshIndices(node);
        }
    }

    void DefaultRenderer::createMeshIndex(std::shared_ptr<Node>& node) {
        if (auto meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            meshes.push_back(meshInstance);
            for(const auto& material : meshInstance->getMesh()->_getMaterials()) {
                if (auto standardMaterial = dynamic_cast<StandardMaterial*>(material.get())) {
                    if (standardMaterial->albedoTexture != nullptr) {
                        textures.insert(standardMaterial->albedoTexture);
                    }
                }
            }
            for(const auto& material : meshInstance->getMesh()->_getMaterials()) {
                if (auto standardMaterial = dynamic_cast<StandardMaterial*>(material.get())) {
                    if (standardMaterial->albedoTexture != nullptr) {
                        auto it = textures.find(standardMaterial->albedoTexture) ;
                        auto index = std::distance(std::begin(textures), it);
                        texturesIndices[standardMaterial->albedoTexture->getId()] = static_cast<int32_t>(index);
                    }
                }
            }
        }
    }

    void DefaultRenderer::loadShaders() {
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void DefaultRenderer::update(float delta) {
        Camera camera{};
        camera.position = { -0.0f, 0.0f, -5.0f };
        camera.setViewTarget({ 0.0f, 0.0f, 0.0f});
        camera.setPerspectiveProjection(glm::radians(50.0f), getAspectRatio(), 0.1f, 100.0f);

        SurfaceUniformBufferObject ubo{
            .projection = camera.getProjection(),
            .view = camera.getView(),
            .inverseView = camera.getInverseView(),
        };

        uint32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            ubo.model = meshInstance->worldTransform;
            if (meshInstance->getMesh()->isValid()) {
                for (const auto &surface: meshInstance->getMesh()->getSurfaces()) {
                    if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                        ubo.albedoColor = standardMaterial->albedoColor.color;
                        if (standardMaterial->albedoTexture == nullptr) {
                            ubo.textureIndex = -1;
                        } else {
                            ubo.textureIndex = texturesIndices[standardMaterial->albedoTexture->getId()];
                        }
                    }
                    writeUniformBuffer(surfacesBuffers, &ubo, surfaceIndex);
                    surfaceIndex += 1;
                }
            }
        }
    }

    void DefaultRenderer::recordCommands(VkCommandBuffer commandBuffer) {
        if (meshes.empty()) return;
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        bindShader(commandBuffer, *vertShader);
        bindShader(commandBuffer, *fragShader);
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
                    bindDescriptorSets(commandBuffer, surfaceIndex);
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    surfaceIndex += 1;
                }
            }
        }
    }

    void DefaultRenderer::createDescriptorSetLayout() {
        if (meshes.empty()) return;
        VkDeviceSize size = sizeof(SurfaceUniformBufferObject);
        uint32_t surfaceCount = 0;
        for (const auto& meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                surfaceCount += meshInstance->getMesh()->getSurfaces().size();
            }
        }
        createUniformBuffers(surfacesBuffers, size, surfaceCount);
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_ALL_GRAPHICS,
                        textures.size())
            .build();
        for (int i = 0; i < surfacesDescriptorSets.size(); i++) {
            auto bufferInfo = surfacesBuffers[i]->descriptorInfo(size);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            for(const auto& texture : textures) {
                imagesInfo.push_back(texture->getImage()._getImage().imageInfo());
            }
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, imagesInfo.data())
                .build(surfacesDescriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

}