#include "z0/vulkan/framebuffers/color_attachment.hpp"
#include "z0/vulkan/framebuffers/color_attachment_hdr.hpp"
#include "z0/log.hpp"

namespace z0 {

    ColorAttachmentHDR::ColorAttachmentHDR(VulkanDevice &dev) : BaseFrameBuffer{dev } {
         createImagesResources();
     }

    void ColorAttachmentHDR::cleanupImagesResources() {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanDevice.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        BaseFrameBuffer::cleanupImagesResources();
    }

    VkDescriptorImageInfo ColorAttachmentHDR::imageInfo() {
        return VkDescriptorImageInfo {
                .sampler = sampler,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    void ColorAttachmentHDR::createImagesResources() {
        createImage(vulkanDevice.getSwapChainExtent().width,
                    vulkanDevice.getSwapChainExtent().height,
                    renderFormat,
                    VK_SAMPLE_COUNT_1_BIT, // Always resolved, only used for post-processing or display
                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(vulkanDevice.getPhysicalDevice(), &properties);
        VkSamplerCreateInfo samplerInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = samplerInfo.addressModeU,
                .addressModeW = samplerInfo.addressModeU,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
                .minLod = 0.0f,
                .maxLod = 1.0f,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
        };
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create color attachment sampler!");
        }

    }

}