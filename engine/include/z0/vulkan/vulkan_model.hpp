#pragma once

#include "z0/vulkan/vulkan_device.hpp"
#include "z0/vulkan/vulkan_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace z0 {
    class VulkanModel {
    public:

        struct Vertex {
            glm::vec2 position{};
            glm::vec3 color{};
            //glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription2EXT> getBindingDescription();
            static std::vector<VkVertexInputAttributeDescription2EXT> getAttributeDescription();

            bool operator==(const Vertex&other) const {
                return position == other.position && color == other.color; // && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
            //void loadModel(const std::string &filepath);
        };

        VulkanModel(VulkanDevice &device, const VulkanModel::Builder &builder);
        ~VulkanModel();

        //static std::unique_ptr<VulkanModel> createModelFromFile(VulkanDevice &device, const std::string &filename);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        VulkanDevice& device;

        std::unique_ptr<VulkanBuffer> vertexBuffer;
        uint32_t vertexCount{0};

        bool hasIndexBuffer {false};
        std::unique_ptr<VulkanBuffer> indexBuffer;
        uint32_t indexCount{0};

        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

    public:
        VulkanModel(const VulkanModel&) = delete;
        VulkanModel &operator=(const VulkanModel&) = delete;
    };
}