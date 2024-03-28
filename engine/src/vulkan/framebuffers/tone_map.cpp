#include "z0/vulkan/framebuffers/tone_map.hpp"

namespace z0 {

    ToneMap::ToneMap(VulkanDevice &dev, VkFormat format) :
            BaseFrameBuffer{dev, format} {
         createImagesResources();
     }

    void ToneMap::createImagesResources() {
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory);
        imageView = vulkanDevice.createImageView(image,
                                                 format,
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                                 1);
    }

}