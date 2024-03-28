#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class BaseFrameBuffer {
    public:
        const VkFormat& getFormat() const { return format; }
        const VkImage& getImage() const { return image; }
        const VkImageView& getImageView() const { return imageView; }

        virtual void createImagesResources() = 0;
        virtual void cleanupImagesResources();

    protected:
        VulkanDevice& vulkanDevice;
        VkFormat format;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;

        BaseFrameBuffer(VulkanDevice &dev, VkFormat fmt): vulkanDevice{dev}, format{fmt} {};

    public:
        BaseFrameBuffer(const BaseFrameBuffer&) = delete;
        BaseFrameBuffer &operator=(const BaseFrameBuffer&) = delete;
        BaseFrameBuffer(const BaseFrameBuffer&&) = delete;
        BaseFrameBuffer &&operator=(const BaseFrameBuffer&&) = delete;
    };

}