#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class VulkanTexture {
    public:
        VulkanTexture(VulkanDevice& device, std::string filepath);
        ~VulkanTexture();

        VkDescriptorImageInfo imageInfo();

    private:
        VulkanDevice& vulkanDevice;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureImage(std::string filepath);
        void createTextureSampler();
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    };

}