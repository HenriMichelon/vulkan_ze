/*
 * Derived from
 * https://vulkan-tutorial.com/Texture_mapping/Images
 */
#include "z0/vulkan/vulkan_image.hpp"
#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/vulkan/vulkan_stats.hpp"
#include "z0/log.hpp"

#include <stb_image.h>
//#include <stb_image_write.h>

namespace z0 {

    VulkanImage::VulkanImage(VulkanDevice& device, uint32_t w, uint32_t h, VkDeviceSize imageSize, void* data):
        vulkanDevice{device}, width{w}, height{h}
    {
        VulkanBuffer textureStagingBuffer{
                vulkanDevice,
                imageSize,
                1,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };
        textureStagingBuffer.map();
        textureStagingBuffer.writeToBuffer(data);

        const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        vulkanDevice.createImage(width, height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Copying-buffer-to-image
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                1
        };
        VkCommandBuffer commandBuffer = vulkanDevice.beginSingleTimeCommands();
        vulkanDevice.transitionImageLayout(commandBuffer,
                textureImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
        vkCmdCopyBufferToImage(
                commandBuffer,
                textureStagingBuffer.getBuffer(),
                textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
        );
        vulkanDevice.endSingleTimeCommands(commandBuffer);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
        textureImageView = vulkanDevice.createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

        generateMipmaps(format);
        createTextureSampler();
#ifdef VULKAN_STATS
        VulkanStats::get().imagesCount += 1;
#endif
    }

    std::shared_ptr<VulkanImage> VulkanImage::createFromFile(VulkanDevice &device, const std::string &filepath) {
        // Create texture image
        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Loading-an-image
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;
        if (!pixels) {
            die("failed to load texture image!");
        }
        auto image =  std::make_shared<VulkanImage>(device, texWidth, texHeight, imageSize, pixels);
        stbi_image_free(pixels);
        return image;
    }

    /*void VulkanImage::saveToFile(VkCommandBuffer commandBuffer, VulkanDevice &device, VkImage image, VkFormat format, int width, int height, const std::string &filepath) {
        VulkanBuffer textureStagingBuffer{
                device,
                calculateImageSize(format, width, height),
                1,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                1
        };
        device.transitionImageLayout(commandBuffer,
                                     image,
                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ,
                                     0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdCopyImageToBuffer(
                commandBuffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                textureStagingBuffer.getBuffer(),
                1,
                &region
        );

        textureStagingBuffer.map();
        stbi_write_png(filepath.c_str(), width, height, 6, textureStagingBuffer.getMappedMemory(), width * 1);

    }

    VkDeviceSize VulkanImage::calculateImageSize(VkFormat format, int width, int height) {
        uint32_t bytesPerPixel = 0;
        switch (format) {
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_SRGB:
                bytesPerPixel = 4;
                break;
            default:
                die("Unsupported image format");
        }
        return width * height * bytesPerPixel;
    }*/

    VulkanImage::~VulkanImage() {
        vkDestroySampler(vulkanDevice.getDevice(), textureSampler, nullptr);
        vkDestroyImageView(vulkanDevice.getDevice(), textureImageView, nullptr);
        vkDestroyImage(vulkanDevice.getDevice(), textureImage, nullptr);
        vkFreeMemory(vulkanDevice.getDevice(), textureImageMemory, nullptr);
    }

    // https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler#page_Updating-the-descriptors
    VkDescriptorImageInfo VulkanImage::imageInfo() {
        return VkDescriptorImageInfo {
            .sampler = textureSampler,
            .imageView = textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    // https://vulkan-tutorial.com/en/Generating_Mipmaps
    void VulkanImage::generateMipmaps(VkFormat imageFormat) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(vulkanDevice.getPhysicalDevice(), imageFormat, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            die("texture image format does not support linear blitting!"); // See todo.txt
        }

        VkCommandBuffer commandBuffer = vulkanDevice.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = textureImage;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = width;
        int32_t mipHeight = height;
        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            vkCmdBlitImage(commandBuffer,
                           textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        vulkanDevice.endSingleTimeCommands(commandBuffer);
    }

    // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
    void VulkanImage::createTextureSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(vulkanDevice.getPhysicalDevice(), &properties);
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE; // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Anisotropy-device-feature
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod =  0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f; // Optional
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            die("failed to create texture sampler!");
        }
    }



}