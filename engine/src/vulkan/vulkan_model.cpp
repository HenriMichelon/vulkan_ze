/*
 * Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Loading_models
*/
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/log.hpp"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
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
            z0::hashCombine(seed, vertex.position); //, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace  z0 {

    VulkanModel::VulkanModel(VulkanDevice &dev, const VulkanModel::Builder &builder):
        device{dev}  {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    VulkanModel::~VulkanModel() {
    }

    std::shared_ptr<VulkanModel> VulkanModel::createFromFile(VulkanDevice &device, const std::string &filename) {
        Builder builder{};
        builder.loadModel(filename);
        return std::make_shared<VulkanModel>(device, builder);
    }

    void VulkanModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at leat 3");
        VkDeviceSize bufferSize = sizeof (vertices[0]) * vertexCount;
        uint32_t  vertexSize = sizeof(vertices[0]);

        VulkanBuffer stagingBuffer {
                device,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        stagingBuffer.map();
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
        hasIndexBuffer = indexCount > 0;
        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof (indices[0]) * indexCount;
        uint32_t  indexSize = sizeof(indices[0]);
        VulkanBuffer stagingBuffer {
                device,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        stagingBuffer.map();
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

    void VulkanModel::draw(VkCommandBuffer commandBuffer) {
        bind(commandBuffer);
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void VulkanModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
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
                offsetof(Vertex, color)
        });
        attributeDescriptions.push_back({
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                2,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(Vertex, normal)
        });
        attributeDescriptions.push_back({
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                3,
                0,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(Vertex, uv)
        });
        return attributeDescriptions;
    }

    // https://fastgltf.readthedocs.io/v0.7.x/overview.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    void VulkanModel::Builder::loadModel(std::filesystem::path filepath) {
        fastgltf::Parser parser;
        fastgltf::GltfDataBuffer data;
        data.loadFromFile(filepath);
        auto asset = parser.loadGltf(&data, filepath.parent_path(), fastgltf::Options::None);
        if (auto error = asset.error(); error != fastgltf::Error::None) {
            die(getErrorMessage(error));
        }
        fastgltf::Asset gltf = std::move(asset.get());

        vertices.clear();
        indices.clear();
        for (fastgltf::Mesh& mesh : gltf.meshes) {
            std::cout << mesh.name << std::endl;
            for (auto&& p : mesh.primitives) {
                size_t initial_vtx = vertices.size();
                // load indexes
                {
                    fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexaccessor.count);
                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                                                             [&](std::uint32_t idx) {
                                                                 indices.push_back(idx + initial_vtx);
                                                             });
                }
                // load vertex positions
                {
                    fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + posAccessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                          [&](glm::vec3 v, size_t index) {
                              Vertex newvtx;
                              newvtx.position = v;
                              newvtx.normal = { 1, 0, 0 };
                              newvtx.color = glm::vec4 { 1.f };
                              newvtx.uv.x = 0;
                              newvtx.uv.y = 0;
                              vertices[initial_vtx + index] = newvtx;
                          });
                }
                // load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                                                                  [&](glm::vec3 v, size_t index) {
                                                                      vertices[initial_vtx + index].normal = v;
                                                                  });
                }
                // load UVs
                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                                                                  [&](glm::vec2 v, size_t index) {
                                                                      vertices[initial_vtx + index].uv.x = v.x;
                                                                      vertices[initial_vtx + index].uv.y = v.y;
                                                                  });
                }
            }
        }
        //die("not implemented");

    }

    /*void VulkanModel::Builder::loadModel(const std::string &filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for(const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                Vertex vertex{};
                if (index.vertex_index >= 0) {
                    vertex.position = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2],
                    };
                    vertex.color = {
                            attrib.colors[3 * index.vertex_index + 0],
                            attrib.colors[3 * index.vertex_index + 1],
                            attrib.colors[3 * index.vertex_index + 2],
                    };
                }
                if (index.normal_index >= 0) {
                    vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2],
                    };
                }
                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }*/
}






