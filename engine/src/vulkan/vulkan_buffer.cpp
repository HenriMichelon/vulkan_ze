/*
 * Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/vulkan/vulkan_stats.hpp"
#include "z0/log.hpp"

#include <cassert>
#include <cstring>

namespace z0 {

/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
    VkDeviceSize VulkanBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    VulkanBuffer::VulkanBuffer(VulkanDevice &vkdevice,
                               VkDeviceSize instanceSize,
                               uint32_t instanceCount,
                               VkBufferUsageFlags usageFlags,
                               VkDeviceSize minOffsetAlignment) :
                vulkanDevice{vkdevice},
                instanceSize{instanceSize},
                instanceCount{instanceCount},
                usageFlags{usageFlags} {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        createBuffer(usageFlags);
#ifdef VULKAN_STATS
        VulkanStats::get().buffersCount += 1;
#endif
    }

    VulkanBuffer::~VulkanBuffer() {
        unmap();
        vmaDestroyBuffer(vulkanDevice.getAllocator(), buffer, allocation);
    }

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @return VkResult of the buffer mapping call
 */
    VkResult VulkanBuffer::map() {
        assert(buffer && allocation && "Called map on buffer before create");
        return vmaMapMemory(vulkanDevice.getAllocator(), allocation, &mapped);
    }

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
    void VulkanBuffer::unmap() {
        if (mapped) {
            vmaUnmapMemory(vulkanDevice.getAllocator(), allocation);
            mapped = nullptr;
        }
    }

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
    void VulkanBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");
        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        } else {
            char *memOffset = (char *)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

/**
 * Flush a memory range of the buffer to make it visible to the vulkanDevice
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
    VkResult VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        return vmaFlushAllocation(vulkanDevice.getAllocator(), allocation, offset, size);
    }

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
    VkResult VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        return vmaInvalidateAllocation(vulkanDevice.getAllocator(), allocation, offset, size);
    }

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
    VkDescriptorBufferInfo VulkanBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{
                buffer,
                offset,
                size,
        };
    }

    void VulkanBuffer::createBuffer(VkBufferUsageFlags usage) {
        const VkBufferCreateInfo bufferInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = bufferSize,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        VmaAllocationCreateInfo allocInfo = {
                .flags = usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT  ?
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT :
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO,
        };
        if (vmaCreateBuffer(vulkanDevice.getAllocator(),
                        &bufferInfo,
                        &allocInfo,
                        &buffer,
                        &allocation,
                        nullptr) != VK_SUCCESS) {
            die("failed to create buffer!");
        }
    }

    void VulkanBuffer::copyTo(VulkanBuffer& dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = vulkanDevice.beginSingleTimeCommands();
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, buffer, dstBuffer.buffer, 1, &copyRegion);
        vulkanDevice.endSingleTimeCommands(commandBuffer);
    }


}
