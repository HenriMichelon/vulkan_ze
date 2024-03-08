#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/nodes/mesh_instance.hpp"

#include <set>

namespace z0 {

    class DefaultRenderer: public VulkanRenderer {
    public:
        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            glm::mat4 inverseView{1.0f};
            int32_t textureIndex{-1};
        };

        DefaultRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~DefaultRenderer();

        void loadScene(const std::shared_ptr<Node>& rootNode);

    private:
        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::shared_ptr<Node> rootNode;
        std::set<std::shared_ptr<Mesh>> meshes {};

        std::map<MeshInstance*, uint32_t> meshesIndices {};
        std::vector<MeshInstance*> meshInstances {};

        void update(float delta) override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;

        void createMeshIndices(const std::shared_ptr<Node>& parent);
        void createMeshIndex(const std::shared_ptr<Node>& node);

    public:
        DefaultRenderer(const DefaultRenderer&) = delete;
        DefaultRenderer &operator=(const DefaultRenderer&) = delete;
        DefaultRenderer(const DefaultRenderer&&) = delete;
        DefaultRenderer &&operator=(const DefaultRenderer&&) = delete;
    };

}