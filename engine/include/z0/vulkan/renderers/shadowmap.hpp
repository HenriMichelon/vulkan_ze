#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class ShadowMap {
    public:
        explicit ShadowMap(VulkanDevice &dev);
        ~ShadowMap();

        const VkFormat format{ VK_FORMAT_D16_UNORM };
#if defined(__ANDROID__)
	    const uint32_t size{ 1024 };
#else
        const uint32_t size{ 2048 };
#endif

        const VkImage& getImage() const { return image; }
        const VkImageView& getImageView() const { return imageView; }
        const VkDeviceMemory& getImageMemory() const { return imageMemory; }
        const VkSampler& getSampler() const { return sampler; }

        void createImagesResources();
        void cleanupImagesResources();

    private:
        VulkanDevice& vulkanDevice;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;
        VkSampler sampler;
    };

}