#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class BaseFrameBuffer {
    public:
        const VkImage& getImage() const { return image; }
        const VkImageView& getImageView() const { return imageView; }

        virtual void createImagesResources() = 0;
        virtual void cleanupImagesResources();

    protected:
        VulkanDevice& vulkanDevice;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;

        BaseFrameBuffer(VulkanDevice &dev): vulkanDevice{dev} {};
        void createImage(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkSampleCountFlagBits samples,
                         VkImageUsageFlags usage,
                         VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT);

    public:
        BaseFrameBuffer(const BaseFrameBuffer&) = delete;
        BaseFrameBuffer &operator=(const BaseFrameBuffer&) = delete;
        BaseFrameBuffer(const BaseFrameBuffer&&) = delete;
        BaseFrameBuffer &&operator=(const BaseFrameBuffer&&) = delete;
    };

}