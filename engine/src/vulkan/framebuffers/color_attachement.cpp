#include "z0/vulkan/framebuffers/color_attachement.hpp"

namespace z0 {

    ColorAttachement::ColorAttachement(VulkanDevice &dev) : BaseFrameBuffer{dev} {
         createImagesResources();
     }

    void ColorAttachement::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    vulkanDevice.getSwapChainImageFormat(),
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    }

}