#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class VulkanImage {
    public:
        VulkanImage(VulkanDevice& device, uint32_t width, uint32_t height, VkDeviceSize imageSize, void* data);
        ~VulkanImage();

        VkDescriptorImageInfo imageInfo();

        static std::shared_ptr<VulkanImage> createFromFile(VulkanDevice &device, const std::string &filepath);
        static void saveToFile(VkCommandBuffer commandBuffer, VulkanDevice &device, VkImage image, VkFormat format, int width, int height, const std::string &filepath);
        static VkDeviceSize calculateImageSize(VkFormat format, int width, int height);

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }

    private:
        uint32_t width, height;

        VulkanDevice& vulkanDevice;
        uint32_t mipLevels;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureSampler();
        void generateMipmaps(VkFormat imageFormat);
    };

}