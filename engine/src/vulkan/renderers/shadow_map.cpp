#include "z0/vulkan/renderers/shadow_map.hpp"
#include "z0/log.hpp"

namespace z0 {

    ShadowMap::ShadowMap(VulkanDevice &dev, SpotLight* spotLight) :
            BaseSharedImage{dev, dev.findImageTilingSupportedFormat(
                    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)},
            light(spotLight) {
         createImagesResources();
     }

    ShadowMap::~ShadowMap() {
        cleanupImagesResources();
    }

    void ShadowMap::createImagesResources() {
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // For shadow mapping we only need a depth attachment
        vulkanDevice.createImage(size, size,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory);
        imageView = vulkanDevice.createImageView(image,
                                                 format,
                                                 VK_IMAGE_ASPECT_DEPTH_BIT,
                                                 1);

        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        VkFilter shadowmap_filter = vulkanDevice.formatIsFilterable( format, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        VkSamplerCreateInfo samplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = shadowmap_filter,
            .minFilter = shadowmap_filter,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = samplerCreateInfo.addressModeU,
            .addressModeW = samplerCreateInfo.addressModeU,
            .mipLodBias = 0.0f,
            .maxAnisotropy = 1.0f,
            .minLod = 0.0f,
            .maxLod = 1.0f,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
        };
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create shadowmap sampler!");
        }

    }

    void ShadowMap::cleanupImagesResources() {
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanDevice.getDevice(), sampler, nullptr);
            vkDestroyImageView(vulkanDevice.getDevice(), imageView, nullptr);
            vkDestroyImage(vulkanDevice.getDevice(), image, nullptr);
            vkFreeMemory(vulkanDevice.getDevice(), imageMemory, nullptr);
            sampler = VK_NULL_HANDLE;
            imageView = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }


}