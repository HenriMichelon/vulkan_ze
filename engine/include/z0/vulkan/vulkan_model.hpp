#pragma once

#include "z0/vulkan/vulkan_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace z0 {

    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};
        bool operator==(const Vertex&other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

    class VulkanModel {
    public:

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
            void loadModel(const std::string &filepath);
        };

        VulkanModel(VulkanDevice &device, const VulkanModel::Builder &builder);
        ~VulkanModel();

        static std::unique_ptr<VulkanModel> createModelFromFile(VulkanDevice &device, const std::string &filename);
        static std::vector<VkVertexInputBindingDescription2EXT> getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription2EXT> getAttributeDescription();

        void draw(VkCommandBuffer commandBuffer);

    private:
        bool hasIndexBuffer {false};
        uint32_t vertexCount{0};
        uint32_t indexCount{0};
        VulkanDevice& device;
        std::unique_ptr<VulkanBuffer> vertexBuffer;
        std::unique_ptr<VulkanBuffer> indexBuffer;

        void bind(VkCommandBuffer commandBuffer);
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

    public:
        VulkanModel(const VulkanModel&) = delete;
        VulkanModel &operator=(const VulkanModel&) = delete;
    };
}