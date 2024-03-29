#include "z0/vulkan/framebuffers/color_attachment.hpp"
#include "z0/vulkan/framebuffers/color_attachment_hdr.hpp"

namespace z0 {

    ColorAttachment::ColorAttachment(VulkanDevice &dev, bool _multisampled) :
        BaseFrameBuffer{dev}, multisampled{_multisampled} {
         createImagesResources();
     }

    // https://vulkan-tutorial.com/Multisampling#page_Setting-up-a-render-target
    void ColorAttachment::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    multisampled ? ColorAttachmentHDR::renderFormat : vulkanDevice.getSwapChainImageFormat(),
                    multisampled ? vulkanDevice.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    }

}