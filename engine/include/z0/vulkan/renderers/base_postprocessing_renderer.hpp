#pragma once

#include "z0/vulkan/renderers/base_renderpass.hpp"
#include "z0/vulkan/framebuffers/color_attachment_hdr.hpp"

namespace z0 {

    class BasePostprocessingRenderer: public BaseRenderpass, public VulkanRenderer {
    public:
        BasePostprocessingRenderer(VulkanDevice& device,
                                   std::string shaderDirectory,
                                   std::shared_ptr<ColorAttachmentHDR>& inputColorAttachmentHdr);

        std::shared_ptr<ColorAttachmentHDR>& getColorAttachment() { return colorAttachmentHdr; }
        VkImage getImage() const override { return colorAttachmentHdr->getImage(); }
        VkImageView getImageView() const override { return colorAttachmentHdr->getImageView(); }

        void cleanup() override;
        void loadShaders() override;
        void createDescriptorSetLayout(VkDeviceSize globalUboSize) ;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

    protected:
        VkDeviceSize globalUboSize;
        std::shared_ptr<ColorAttachmentHDR> colorAttachmentHdr;
        std::shared_ptr<ColorAttachmentHDR> inputColorAttachmentHdr;
    };

}