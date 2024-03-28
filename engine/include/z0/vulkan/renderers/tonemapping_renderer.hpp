#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/vulkan_cubemap.hpp"
#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {

    class TonemappingRenderer: public BaseRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float gamma{2.2};
        };

        TonemappingRenderer(VulkanDevice& device, std::string shaderDirectory);

        void cleanup();
        void update(Camera* currentCamera, uint32_t currentFrame);
        void loadShaders();
        void createDescriptorSetLayout() ;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);

    private:
    };

}