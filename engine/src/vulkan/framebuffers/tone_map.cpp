#include "z0/vulkan/framebuffers/tone_map.hpp"
#include "z0/vulkan/framebuffers/multisampled.hpp"

namespace z0 {

    ToneMap::ToneMap(VulkanDevice &dev) : BaseFrameBuffer{dev } {
         createImagesResources();
     }

    void ToneMap::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    Multisampled::renderFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    }

}