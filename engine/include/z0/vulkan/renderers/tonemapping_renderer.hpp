#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/framebuffers/tone_map.hpp"
#include "z0/vulkan/framebuffers/color_attachement.hpp"

namespace z0 {

    class TonemappingRenderer: public BaseRenderer, public VulkanRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float gamma{2.2};
            alignas(4) float exposure{1.0};
        };

        TonemappingRenderer(VulkanDevice& device, std::string shaderDirectory);

        std::shared_ptr<ToneMap>& getToneMap() { return toneMap; }

        void cleanup();
        void update(uint32_t currentFrame);
        void loadShaders();
        void createDescriptorSetLayout() ;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);
        void beginRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage, VkImageView swapChainImageView);
        void endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage);
        void createImagesResources();
        void cleanupImagesResources();
        void recreateImagesResources();

    private:
        std::shared_ptr<ToneMap> toneMap;
    };

}