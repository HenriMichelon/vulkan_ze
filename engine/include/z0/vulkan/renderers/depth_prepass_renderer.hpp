#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/renderers/depth_buffer.hpp"
#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    class DepthPressPassRenderer: public BaseRenderer {
    public:
        struct GobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
        };
        struct ModelUniformBufferObject {
            glm::mat4 matrix;
        };

        DepthPressPassRenderer(VulkanDevice& device, const std::string& shaderDirectory);

        void loadScene(std::vector<MeshInstance*>& meshes);
        void cleanup() override;

    private:
        std::vector<MeshInstance*> meshes {};
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) override;

    public:
        DepthPressPassRenderer(const DepthPressPassRenderer&) = delete;
        DepthPressPassRenderer &operator=(const DepthPressPassRenderer&) = delete;
        DepthPressPassRenderer(const DepthPressPassRenderer&&) = delete;
        DepthPressPassRenderer &&operator=(const DepthPressPassRenderer&&) = delete;
    };

}