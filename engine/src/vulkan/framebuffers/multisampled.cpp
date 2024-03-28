#include "z0/vulkan/framebuffers/multisampled.hpp"

namespace z0 {

    Multisampled::Multisampled(VulkanDevice &dev, VkFormat format) :
            BaseFrameBuffer{dev, format} {
         createImagesResources();
     }

    void Multisampled::createImagesResources() {
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height,
                                 1,
                                 vulkanDevice.getSamples(),
                                 format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory);
        imageView = vulkanDevice.createImageView(image,
                                                 format,
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                                 1);
    }

}