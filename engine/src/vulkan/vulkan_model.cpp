/*
 * Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Loading_models
*/
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/log.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <unordered_map>

namespace  z0 {
    // from: https://stackoverflow.com/a/57595105
    template<typename T, typename... Rest>
    void hashCombine(std::size_t &seed, const T &v, const Rest &... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };
}

namespace std {
    template<>
    struct hash<z0::Vertex>{
        size_t operator()(z0::Vertex const &vertex) const {
            size_t seed = 0;
            z0::hashCombine(seed, vertex.position, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace  z0 {

    VulkanModel::VulkanModel(VulkanDevice &dev, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices):
        device{dev}  {
        createVertexBuffers(vertices);
        createIndexBuffers(indices);
    }

    void VulkanModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at leat 3");
        VkDeviceSize bufferSize = sizeof (vertices[0]) * vertexCount;
        uint32_t  vertexSize = sizeof(vertices[0]);

        const VulkanBuffer stagingBuffer {
                device,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        stagingBuffer.writeToBuffer((void*)vertices.data());

        vertexBuffer = std::make_unique<VulkanBuffer>(
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                );
        stagingBuffer.copyTo(*vertexBuffer, bufferSize);
    }

    void VulkanModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        if (indexCount <= 0) {
            die("Unindexed meshes aren't supported");
        }
        VkDeviceSize bufferSize = sizeof (indices[0]) * indexCount;
        uint32_t  indexSize = sizeof(indices[0]);
        const VulkanBuffer stagingBuffer {
                device,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };
        stagingBuffer.writeToBuffer((void*)indices.data());

        indexBuffer = std::make_unique<VulkanBuffer>(
                device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                );
        stagingBuffer.copyTo(*indexBuffer, bufferSize);
    }

    void VulkanModel::draw(VkCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t count, uint32_t firstInstance) {
        bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer, count, 1, firstIndex, 0, firstInstance);
    }

    void VulkanModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    std::vector<VkVertexInputBindingDescription2EXT> VulkanModel::getBindingDescription() {
        std::vector<VkVertexInputBindingDescription2EXT> bindingDescriptions(1);
        bindingDescriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions[0].divisor = 1;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription2EXT> VulkanModel::getAttributeDescription() {
        std::vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions{};
        attributeDescriptions.push_back({
            VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            nullptr,
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, position)
        });
        attributeDescriptions.push_back({
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                1,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(Vertex, normal)
        });
        attributeDescriptions.push_back({
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                2,
                0,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(Vertex, uv)
        });
        attributeDescriptions.push_back({
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                3,
                0,
                VK_FORMAT_R32G32B32A32_SFLOAT,
                offsetof(Vertex, tangent)
        });
        return attributeDescriptions;
    }

}






