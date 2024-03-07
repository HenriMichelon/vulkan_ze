#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/mesh.hpp"

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
                        std::vector<std::shared_ptr<Mesh>>& models);
        ~DefaultRenderer();

    private:
        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::vector<std::shared_ptr<Mesh>> models;

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