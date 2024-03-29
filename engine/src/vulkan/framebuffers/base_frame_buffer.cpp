#include "z0/vulkan/framebuffers/base_frame_buffer.hpp"

namespace z0 {

    void BaseFrameBuffer::cleanupImagesResources() {
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroyImageView(vulkanDevice.getDevice(), imageView, nullptr);
            vkDestroyImage(vulkanDevice.getDevice(), image, nullptr);
            vkFreeMemory(vulkanDevice.getDevice(), imageMemory, nullptr);
            imageView = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }

    void BaseFrameBuffer::createImage(uint32_t width,
                                      uint32_t height,
                                      VkFormat format,
                                      VkSampleCountFlagBits samples,
                                      VkImageUsageFlags usage,
                                      VkImageAspectFlags flags) {
        vulkanDevice.createImage(width,
                                 height,
                                 1,
                                 samples,
                                 format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 usage,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory);
        imageView = vulkanDevice.createImageView(image, format,flags,1);
    }

}