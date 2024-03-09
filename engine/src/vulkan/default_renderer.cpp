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
        loadResources();
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
            for(const auto& material : meshInstance->getMesh()->getMaterials()) {
                if (material->albedoTexture != nullptr) {
                   textures.insert(material->albedoTexture);
                }
            }
            for(const auto& material : meshInstance->getMesh()->getMaterials()) {
                if (material->albedoTexture != nullptr) {
                    auto it = textures.find(material->albedoTexture) ;
                    auto index = std::distance(std::begin(textures), it);
                    texturesIndices[material->albedoTexture->getId()] = static_cast<int32_t>(index);
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
        camera.transform.position = { -0.0f, 0.0f, -5.0f };
        camera.setViewYXZ();
        camera.setPerspectiveProjection(glm::radians(50.0f), getAspectRatio(), 0.1f, 100.0f);

        UniformBufferObject ubo{
            .projection = camera.getProjection(),
            .view = camera.getView(),
            .inverseView = camera.getInverseView(),
        };

        uint32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                ubo.model = meshInstance->transform.mat4();
                for (const auto &surface: meshInstance->getMesh()->getSurfaces()) {
                    auto material = meshInstance->getMesh()->getMaterials()[surface->materialIndex];
                    if (material->albedoTexture == nullptr) {
                        ubo.textureIndex = -1;
                    } else {
                        ubo.textureIndex = texturesIndices[material->albedoTexture->getId()];
                    }
                    writeUniformBuffer(&ubo, surfaceIndex);
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
            if (meshInstance->getMesh()->isValid()) {
                for (const auto& surface: meshInstance->getMesh()->getSurfaces()) {
                    auto material = meshInstance->getMesh()->getMaterials()[surface->materialIndex];
                    vkCmdSetCullMode(commandBuffer,
                                     material->cullMode == CULLMODE_DISABLED ? VK_CULL_MODE_NONE :
                                     material->cullMode == CULLMODE_BACK ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT);
                    bindDescriptorSets(commandBuffer, surfaceIndex);
                    surface->_model->draw(commandBuffer);
                    surfaceIndex += 1;
                }
            }
        }
    }

    void DefaultRenderer::createDescriptorSetLayout() {
        if (meshes.empty()) return;
        VkDeviceSize size = sizeof(UniformBufferObject);
        uint32_t surfaceCount = 0;
        for (const auto& meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                surfaceCount += meshInstance->getMesh()->getSurfaces().size();
            }
        }
        createUniformBuffers(size, surfaceCount);
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_ALL_GRAPHICS,
                        textures.size())
            .build();
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo(size);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            for(const auto& texture : textures) {
                imagesInfo.push_back(texture->getImage()._getImage().imageInfo());
            }
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, imagesInfo.data())
                .build(globalDescriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

}