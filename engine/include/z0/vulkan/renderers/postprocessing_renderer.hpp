#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/framebuffers/color_attachment_hdr.hpp"
#include "z0/vulkan/framebuffers/color_attachment.hpp"

namespace z0 {

    class PostprocessingRenderer: public BaseRenderer, public VulkanRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float dummy;
        };

        PostprocessingRenderer(VulkanDevice& device, std::string shaderDirectory, std::shared_ptr<ColorAttachmentHDR> inputColorAttachmentHdr);

        std::shared_ptr<ColorAttachmentHDR>& getColorAttachment() { return colorAttachmentHdr; }

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
        std::shared_ptr<ColorAttachmentHDR> colorAttachmentHdr;
        std::shared_ptr<ColorAttachmentHDR> inputColorAttachmentHdr;
    };

}