#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/vulkan_cubemap.hpp"
#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {

    class SkyboxRenderer: public BaseRenderer {
    public:
        struct GobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
        };

        SkyboxRenderer(VulkanDevice& device, std::string shaderDirectory);

        void cleanup();
        void loadScene(std::shared_ptr<VulkanCubemap>& cubemap, Camera* camera);
        void update(uint32_t currentFrame);
        void loadShaders();
        void createDescriptorSetLayout() ;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);

    private:
        uint32_t vertexCount;
        Camera* currentCamera {nullptr};
        std::shared_ptr<VulkanCubemap> cubemap;
        std::unique_ptr<VulkanBuffer> vertexBuffer;
    };

}