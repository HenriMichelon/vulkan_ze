#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/framebuffers/tone_map.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {

    class TonemappingRenderer: public BaseRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float gamma;
        };

        TonemappingRenderer(VulkanDevice& device, std::string shaderDirectory, ToneMap& toneMap);

        void cleanup();
        void update(uint32_t currentFrame);
        void loadShaders();
        void createDescriptorSetLayout() ;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);

    private:
        ToneMap& toneMap;
    };

}