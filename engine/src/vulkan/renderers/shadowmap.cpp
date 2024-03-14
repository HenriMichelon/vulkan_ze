#include "z0/vulkan/renderers/shadowmap.hpp"
#include "z0/log.hpp"

namespace z0 {

    ShadowMap::ShadowMap(VulkanDevice &dev, SpotLight* spotLight) :
        vulkanDevice{dev}, light(spotLight) {
         createImagesResources();
     }

    ShadowMap::~ShadowMap() {
        cleanupImagesResources();
    }

    void ShadowMap::createImagesResources() {
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // For shadow mapping we only need a depth attachment
        /*vulkanDevice.createImage(size, size,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory);
        imageView = vulkanDevice.createImageView(image,
                                                 format,
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                                 1);*/

        vulkanDevice.createImage(size, size,
                                 1,
                                 vulkanDevice.getSamples(),
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
        VkSamplerCreateInfo samplerCreateInfo {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.magFilter = shadowmap_filter;
        samplerCreateInfo.minFilter = shadowmap_filter;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
        samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create shadowmap sampler!");
        }
/*
        imageBlit.srcOffsets[0] = {0, 0, 0 };
        imageBlit.srcOffsets[1] = {
                static_cast<int32_t>(size),
                static_cast<int32_t>(size), 1 };
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.mipLevel = 0;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.dstOffsets[0] = {0, 0, 0 };
        imageBlit.dstOffsets[1] = {
                static_cast<int32_t>(size),
                static_cast<int32_t>(size), 1 };
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.mipLevel = 0;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.dstSubresource.layerCount = 1;

        imageResolve.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        imageResolve.srcOffset = {0, 0, 0};
        imageResolve.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        imageResolve.dstOffset = {0, 0, 0};
        imageResolve.extent = {size,size,1};*/
    }

    void ShadowMap::cleanupImagesResources() {
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanDevice.getDevice(), sampler, nullptr);
            /*vkDestroyImageView(vulkanDevice.getDevice(), imageViewMultisampled, nullptr);
            vkDestroyImage(vulkanDevice.getDevice(), imageMultisampled, nullptr);
            vkFreeMemory(vulkanDevice.getDevice(), imageMemoryMultisampled, nullptr);*/
            vkDestroyImageView(vulkanDevice.getDevice(), imageView, nullptr);
            vkDestroyImage(vulkanDevice.getDevice(), image, nullptr);
            vkFreeMemory(vulkanDevice.getDevice(), imageMemory, nullptr);
            sampler = VK_NULL_HANDLE;
            /*imageViewMultisampled = VK_NULL_HANDLE;
            imageMultisampled = VK_NULL_HANDLE;
            imageMemoryMultisampled = VK_NULL_HANDLE;*/
            imageView = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }


}