#pragma once

#include <set>
#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    class DefaultRenderer: public VulkanRenderer {
    public:
        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
            glm::mat4 inverseView{1.0f};
            uint32_t textureBinding;
        };

        DefaultRenderer(VulkanDevice& device, const std::string& shaderDirectory,
                        std::vector<std::shared_ptr<MeshInstance>>& models);
        ~DefaultRenderer();

    private:
        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::vector<std::shared_ptr<MeshInstance>> nodes;
        std::set<std::shared_ptr<Mesh>> meshes {};
        std::map<std::shared_ptr<MeshInstance>, uint32_t> meshesIndices {};

        void update(float delta) override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;

    public:
        DefaultRenderer(const DefaultRenderer&) = delete;
        DefaultRenderer &operator=(const DefaultRenderer&) = delete;
        DefaultRenderer(const DefaultRenderer&&) = delete;
        DefaultRenderer &&operator=(const DefaultRenderer&&) = delete;
    };

}