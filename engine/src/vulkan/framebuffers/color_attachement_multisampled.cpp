#include "z0/vulkan/framebuffers/color_attachement_multisampled.hpp"

namespace z0 {

    ColorAttachementMultisampled::ColorAttachementMultisampled(VulkanDevice &dev) : BaseFrameBuffer{dev} {
         createImagesResources();
     }

    // https://vulkan-tutorial.com/Multisampling#page_Setting-up-a-render-target
    void ColorAttachementMultisampled::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    renderFormat,
                    vulkanDevice.getSamples(),
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    }

}