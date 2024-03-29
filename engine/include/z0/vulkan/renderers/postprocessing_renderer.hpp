#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/framebuffers/color_attachement_hdr.hpp"
#include "z0/vulkan/framebuffers/color_attachement.hpp"

namespace z0 {

    class PostprocessingRenderer: public BaseRenderer, public VulkanRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float gamma{2.2};
            alignas(4) float exposure{1.0};
        };

        PostprocessingRenderer(VulkanDevice& device, std::string shaderDirectory);

        std::shared_ptr<ColorAttachementHDR>& getToneMap() { return colorAttachementHdr; }

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
        std::shared_ptr<ColorAttachementHDR> colorAttachementHdr;
    };

}