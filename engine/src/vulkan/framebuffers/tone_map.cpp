#include "z0/vulkan/framebuffers/tone_map.hpp"
#include "z0/vulkan/framebuffers/color_attachement_multisampled.hpp"
#include "z0/log.hpp"

namespace z0 {

    ToneMap::ToneMap(VulkanDevice &dev) : BaseFrameBuffer{dev } {
         createImagesResources();
     }

    void ToneMap::cleanupImagesResources() {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanDevice.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        BaseFrameBuffer::cleanupImagesResources();
    }

    VkDescriptorImageInfo ToneMap::imageInfo() {
        return VkDescriptorImageInfo {
                .sampler = sampler,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    void ToneMap::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    ColorAttachementMultisampled::renderFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(vulkanDevice.getPhysicalDevice(), &properties);
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod =  0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.mipLodBias = 0.0f;
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create tone mapping sampler!");
        }

    }

}