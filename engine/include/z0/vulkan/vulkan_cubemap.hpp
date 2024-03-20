#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class VulkanCubemap {
    public:
        VulkanCubemap(VulkanDevice& device, uint32_t width, uint32_t height, VkDeviceSize imageSize, std::vector<void*>& data);
        ~VulkanCubemap();

        VkDescriptorImageInfo imageInfo();
        VkImage& getImage() { return textureImage; }
        VkImageView& getImageView() { return textureImageView; }

        static std::shared_ptr<VulkanCubemap> createFromFile(VulkanDevice &device, const std::string &filepath, const std::string &ext);

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }

    private:
        uint32_t width, height;

        VulkanDevice& vulkanDevice;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureSampler();
    };

}