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

        VkSamplerCreateInfo samplerCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW =VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.0f,
                .maxAnisotropy = 1.0f,
                .minLod = 0.0f,
                .maxLod = 1.0f,
                .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK
        };
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create tone mapping sampler!");
        }

    }

}