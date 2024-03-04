#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class VulkanImage {
    public:
        VulkanImage(VulkanDevice& device, std::string filepath);
        ~VulkanImage();

    private:
        VulkanDevice& vulkanDevice;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;

        void createTextureImage(std::string filepath);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    };

}