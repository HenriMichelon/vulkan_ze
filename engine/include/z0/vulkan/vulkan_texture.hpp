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
        uint32_t mipLevels;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureImage(std::string filepath);
        void createTextureSampler();
        void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    };

}