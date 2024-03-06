#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"

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

        DefaultRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~DefaultRenderer();

    private:
        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::unique_ptr<VulkanModel> model;
        std::unique_ptr<VulkanTexture> texture;
        std::unique_ptr<VulkanModel> model1;
        std::unique_ptr<VulkanTexture> texture1;

        void update(float delta) override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadModels() override;
        void loadShaders() override;

    public:
        DefaultRenderer(const DefaultRenderer&) = delete;
        DefaultRenderer &operator=(const DefaultRenderer&) = delete;
        DefaultRenderer(const DefaultRenderer&&) = delete;
        DefaultRenderer &&operator=(const DefaultRenderer&&) = delete;
    };

}