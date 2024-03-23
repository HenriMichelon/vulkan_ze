#pragma once

#include "z0/vulkan/vulkan_device.hpp"

#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"

namespace z0 {

    class VulkanBuffer {
    public:
        VulkanBuffer(VulkanDevice& vulkanDevice,
                     VkDeviceSize instanceSize,
                     uint32_t instanceCount,
                     VkBufferUsageFlags usageFlags,
                     VkDeviceSize minOffsetAlignment = 1);
        ~VulkanBuffer();

        VkBuffer getBuffer() const { return buffer; }
        VkDeviceSize getAlignmentSize() const { return alignmentSize; }
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        VkResult map();
        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        void copyTo(VulkanBuffer& dstBuffer, VkDeviceSize size) const;

    private:
        VulkanDevice& vulkanDevice;
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkDeviceSize bufferSize;
        VkDeviceSize alignmentSize;
        void* mapped = nullptr;

    public:
        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;
        VulkanBuffer(const VulkanBuffer&&) = delete;
        VulkanBuffer&& operator=(const VulkanBuffer&&) = delete;
    };

}
