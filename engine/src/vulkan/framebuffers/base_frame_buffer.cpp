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

}