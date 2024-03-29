#include "z0/vulkan/framebuffers/color_attachement.hpp"
#include "z0/vulkan/framebuffers/color_attachement_multisampled.hpp"

namespace z0 {

    ColorAttachement::ColorAttachement(VulkanDevice &dev) : BaseFrameBuffer{dev} {
         createImagesResources();
     }

    ColorAttachement::~ColorAttachement() {
        cleanupImagesResources();
    }

    void ColorAttachement::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    ColorAttachementMultisampled::renderFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    }

}