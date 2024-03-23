/*
 * https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
 * https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/
 *
 */
#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/vulkan/vulkan_stats.hpp"
#include "z0/log.hpp"

namespace z0 {

    VulkanBuffer::VulkanBuffer(VulkanDevice &vkdevice,
                               VkDeviceSize instanceSize,
                               uint32_t instanceCount,
                               VkBufferUsageFlags usageFlags,
                               VkDeviceSize minOffsetAlignment) : vulkanDevice{vkdevice} {
        alignmentSize = minOffsetAlignment > 0 ? (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1) : instanceSize;
        bufferSize = alignmentSize * instanceCount;
        const VkBufferCreateInfo bufferInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = bufferSize,
                .usage = usageFlags,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        VmaAllocationCreateInfo allocInfo = {
                .flags = usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT  ?
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
#ifdef VULKAN_STATS
        VulkanStats::get().buffersCount += 1;
#endif
    }

    VulkanBuffer::~VulkanBuffer() {
        if (mapped) {
            vmaUnmapMemory(vulkanDevice.getAllocator(), allocation);
            mapped = nullptr;
        }
        vmaDestroyBuffer(vulkanDevice.getAllocator(), buffer, allocation);
    }

    VkResult VulkanBuffer::map() {
        return vmaMapMemory(vulkanDevice.getAllocator(), allocation, &mapped);
    }

    void VulkanBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) const {
        if (size == VK_WHOLE_SIZE) {
            vmaCopyMemoryToAllocation(vulkanDevice.getAllocator(),
                                      data,
                                      allocation,
                                      0,
                                      bufferSize);
        } else {
            vmaCopyMemoryToAllocation(vulkanDevice.getAllocator(),
                                      data,
                                      allocation,
                                      offset,
                                      size);
        }
    }

    VkDescriptorBufferInfo VulkanBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) const {
        return VkDescriptorBufferInfo{
                buffer,
                offset,
                size,
        };
    }

    void VulkanBuffer::copyTo(VulkanBuffer& dstBuffer, VkDeviceSize size) const {
        VkCommandBuffer commandBuffer = vulkanDevice.beginSingleTimeCommands();
        const VkBufferCopy copyRegion{
            .size = size
        };
        vkCmdCopyBuffer(commandBuffer, buffer, dstBuffer.buffer, 1, &copyRegion);
        vulkanDevice.endSingleTimeCommands(commandBuffer);
    }


}
