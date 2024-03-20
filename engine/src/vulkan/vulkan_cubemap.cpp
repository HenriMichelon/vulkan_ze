/*
 */
#include "z0/vulkan/vulkan_cubemap.hpp"
#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/log.hpp"
#include "z0/vulkan/vulkan_stats.hpp"

#include <stb_image.h>
#include <array>

namespace z0 {

    VulkanCubemap::VulkanCubemap(VulkanDevice& device, uint32_t w, uint32_t h, VkDeviceSize imageSize, std::vector<void*>& data):
        vulkanDevice{device}, width{w}, height{h}
    {
        VulkanBuffer textureStagingBuffer{
                vulkanDevice,
                imageSize,
                6,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                vulkanDevice.getDeviceProperties().limits.minUniformBufferOffsetAlignment
        };
        textureStagingBuffer.map();
        for (int i = 0; i < 6; i++) {
            textureStagingBuffer.writeToBuffer(data[i], imageSize, textureStagingBuffer.getAlignmentSize() * i);
        }

        const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        vulkanDevice.createImage(width, height, 1, VK_SAMPLE_COUNT_1_BIT, format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory,
                                 VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, 6);

        vulkanDevice.transitionImageLayout(
                textureImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

        VkBufferImageCopy layerRegions[6];
        memset(layerRegions, 0, sizeof(layerRegions));
        for (uint32_t i = 0; i < 6; i++) {
            layerRegions[i].bufferOffset = textureStagingBuffer.getAlignmentSize() * i;
            layerRegions[i].bufferRowLength = 0;   // Tightly packed
            layerRegions[i].bufferImageHeight = 0; // Tightly packed
            layerRegions[i].imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = i,
                .layerCount = 1,
            };
            layerRegions[i].imageOffset = {0, 0, 0};
            layerRegions[i].imageExtent = {
                width,
                height,
                1
            };
        }
        VkCommandBuffer commandBuffer = vulkanDevice.beginSingleTimeCommands();
        vkCmdCopyBufferToImage(
                commandBuffer,
                textureStagingBuffer.getBuffer(),
                textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                6,
                layerRegions
        );
        vulkanDevice.endSingleTimeCommands(commandBuffer);

        vulkanDevice.transitionImageLayout(
                textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

        textureImageView = vulkanDevice.createImageView(textureImage, format,
                                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                                        1,
                                                        VK_IMAGE_VIEW_TYPE_CUBE);
        createTextureSampler();

#ifdef VULKAN_STATS
        VulkanStats::get().imagesCount += 1;
#endif
    }

    std::shared_ptr<VulkanCubemap> VulkanCubemap::createFromFile(VulkanDevice &device, const std::string &filepath, const std::string &ext) {
        int texWidth, texHeight, texChannels;
        std::vector<void*> data;
        const std::array<std::string, 6> names { "front", "back", "top", "bottom", "right", "left" };
        for (int i = 0; i < 6; i++) {
            std::string path = filepath + "_" + names[i] + ext;
            stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels) {
                die("failed to load texture image", path);
            }
            data.push_back(pixels);
        }
        VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;
        auto cubemap =  std::make_shared<VulkanCubemap>(device, texWidth, texHeight, imageSize, data);
        for (int i = 0; i < 6; i++) {
            stbi_image_free(data[i]);
        }
        return cubemap;
    }

    VulkanCubemap::~VulkanCubemap() {
        vkDestroySampler(vulkanDevice.getDevice(), textureSampler, nullptr);
        vkDestroyImageView(vulkanDevice.getDevice(), textureImageView, nullptr);
        vkDestroyImage(vulkanDevice.getDevice(), textureImage, nullptr);
        vkFreeMemory(vulkanDevice.getDevice(), textureImageMemory, nullptr);
    }

    VkDescriptorImageInfo VulkanCubemap::imageInfo() {
        return VkDescriptorImageInfo {
            .sampler = textureSampler,
            .imageView = textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    void VulkanCubemap::createTextureSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(vulkanDevice.getPhysicalDevice(), &properties);
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // This ensures that the edges between the faces of the cubemap are seamlessly sampled.
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // "
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // "
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
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            die("failed to create texture sampler!");
        }
    }



}