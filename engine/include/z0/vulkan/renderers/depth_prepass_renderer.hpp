#pragma once

#include "z0/vulkan/renderers/base_meshes_renderer.hpp"

namespace z0 {

    class DepthPrepassRenderer: public BaseMeshesRenderer {
    public:
        struct GlobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
        };
        struct ModelUniformBufferObject {
            glm::mat4 matrix;
        };
        struct SurfaceUniformBufferObject {
            alignas(4) bool transparency;
            alignas(4) int32_t diffuseIndex{-1};
            alignas(16) glm::vec4 albedoColor;
        };

        DepthPrepassRenderer(VulkanDevice& device, const std::string& shaderDirectory);

        void loadScene(std::shared_ptr<DepthBuffer>& buffer,
                       Camera* camera,
                       std::vector<MeshInstance*>& meshes,
                       std::map<Resource::rid_t, int32_t>& imagesIndices,
                       std::unordered_set<std::shared_ptr<VulkanImage>>& images);

    private:
        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) override;

    };

}