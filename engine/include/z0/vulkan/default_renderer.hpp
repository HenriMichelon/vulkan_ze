#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/nodes/mesh_instance.hpp"

#include <set>

namespace z0 {

    class DefaultRenderer: public VulkanRenderer {
    public:
        struct GobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            alignas(16) glm::vec3 directionToLight = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
            alignas(16) glm::vec4 ambientLightColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGB + Intensity
        };
        struct ModelUniformBufferObject {
            glm::mat4 matrix;
            glm::mat4 normalMatrix;
        };
        struct SurfaceUniformBufferObject {
            alignas(16) int32_t textureIndex{-1};
            alignas(16) glm::vec4 albedoColor;
        };

        DefaultRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~DefaultRenderer();

        void loadScene(const std::shared_ptr<Node>& rootNode);

    private:
        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::shared_ptr<Node> rootNode;
        std::vector<MeshInstance*> meshes {};
        std::unordered_set<std::shared_ptr<Texture>> textures {};
        std::map<Resource::rid_t, int32_t> texturesIndices {};

        std::unique_ptr<VulkanDescriptorPool> globalPool {};
        std::vector<std::unique_ptr<VulkanBuffer>> globalBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> surfacesBuffers{MAX_FRAMES_IN_FLIGHT};

        void update(float delta) override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;

        void createMeshIndices(std::shared_ptr<Node>& parent);
        void createMeshIndex(std::shared_ptr<Node>& node);

    public:
        DefaultRenderer(const DefaultRenderer&) = delete;
        DefaultRenderer &operator=(const DefaultRenderer&) = delete;
        DefaultRenderer(const DefaultRenderer&&) = delete;
        DefaultRenderer &&operator=(const DefaultRenderer&&) = delete;
    };

}