#pragma once

#include "z0/vulkan/renderers/base_meshes_renderer.hpp"

namespace z0 {

    class DepthPrepassRenderer: public BaseMeshesRenderer {
    public:
        struct GlobalUniform {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
        };
        struct ModelUniform {
            glm::mat4 matrix;
        };

        DepthPrepassRenderer(VulkanDevice& device, const std::string& shaderDirectory);

        void loadScene(std::shared_ptr<DepthBuffer>& buffer,
                       std::shared_ptr<Camera>& camera,
                       std::vector<std::shared_ptr<MeshInstance>>& meshes);

    private:
        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

    };

}