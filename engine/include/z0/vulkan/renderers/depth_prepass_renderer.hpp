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

        DepthPrepassRenderer(VulkanDevice& device, const std::string& shaderDirectory);

        void loadScene(std::shared_ptr<DepthBuffer>& buffer,
                       Camera* camera,
                       std::vector<MeshInstance*>& meshes);

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