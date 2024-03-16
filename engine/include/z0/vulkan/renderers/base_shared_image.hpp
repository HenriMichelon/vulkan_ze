#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class BaseSharedImage {
    public:

        const VkFormat& getFormat() const { return format; }
        const VkImage& getImage() const { return image; }
        const VkImageView& getImageView() const { return imageView; }

    protected:
        VulkanDevice& vulkanDevice;
        VkFormat format;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;

        BaseSharedImage(VulkanDevice &dev, VkFormat fmt): vulkanDevice{dev}, format{fmt} {};

    public:
        BaseSharedImage(const BaseSharedImage&) = delete;
        BaseSharedImage &operator=(const BaseSharedImage&) = delete;
        BaseSharedImage(const BaseSharedImage&&) = delete;
        BaseSharedImage &&operator=(const BaseSharedImage&&) = delete;
    };

}