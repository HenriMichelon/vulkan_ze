#pragma once

#include "z0/vulkan/vulkan_buffer.hpp"
#include "z0/vertex.hpp"

#include <memory>
#include <vector>
#include <filesystem>

namespace z0 {

    class VulkanModel {
    public:
        VulkanModel(VulkanDevice &device, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

        static std::vector<VkVertexInputBindingDescription2EXT> getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription2EXT> getAttributeDescription();

        void draw(VkCommandBuffer commandBuffer, uint32_t first, uint32_t count);

    private:
        VulkanDevice& device;
        uint32_t vertexCount{0};
        uint32_t indexCount{0};
        std::unique_ptr<VulkanBuffer> vertexBuffer;
        std::unique_ptr<VulkanBuffer> indexBuffer;

        void bind(VkCommandBuffer commandBuffer);
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

    public:
        VulkanModel(const VulkanModel&) = delete;
        VulkanModel &operator=(const VulkanModel&) = delete;
        VulkanModel(const VulkanModel&&) = delete;
        VulkanModel &&operator=(const VulkanModel&&) = delete;
    };
}