#pragma once

#include "z0/vulkan/vulkan_device.hpp"
#include "z0/nodes/spot_light.hpp"

namespace z0 {

    class ShadowMap {
    public:
        explicit ShadowMap(VulkanDevice &dev, SpotLight* spotLight);
        ~ShadowMap();

        const VkFormat format{ VK_FORMAT_D32_SFLOAT };
#if defined(__ANDROID__)
	    const uint32_t size{ 1024 };
#else
        const uint32_t size{ 2048 };
#endif

        const VkImage& getImage() const { return image; }
        const VkImageView& getImageView() const { return imageView; }
        const VkDeviceMemory& getImageMemory() const { return imageMemory; }
        const VkSampler& getSampler() const { return sampler; }
        SpotLight* getLight() { return light; }

        void createImagesResources();
        void cleanupImagesResources();

        /*VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;*/
    private:
        SpotLight* light;
        VulkanDevice& vulkanDevice;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;
        VkSampler sampler;

    public:
        ShadowMap(const ShadowMap&) = delete;
        ShadowMap &operator=(const ShadowMap&) = delete;
        ShadowMap(const ShadowMap&&) = delete;
        ShadowMap &&operator=(const ShadowMap&&) = delete;
    };

}