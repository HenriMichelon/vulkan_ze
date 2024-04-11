#pragma once

#include "z0/vulkan/renderers/base_renderpass.hpp"
#include "z0/vulkan/vulkan_cubemap.hpp"
#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {

    class SkyboxRenderer: public BaseRenderpass {
    public:
        struct GobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
        };

        SkyboxRenderer(VulkanDevice& device, std::string shaderDirectory);

        void cleanup() override;
        void loadScene(std::shared_ptr<VulkanCubemap>& cubemap);
        void update(std::shared_ptr<Camera> currentCamera, uint32_t currentFrame);
        void loadShaders() override;
        void createDescriptorSetLayout() override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

    private:
        uint32_t vertexCount;
        std::shared_ptr<VulkanCubemap> cubemap;
        std::unique_ptr<VulkanBuffer> vertexBuffer;
    };

}