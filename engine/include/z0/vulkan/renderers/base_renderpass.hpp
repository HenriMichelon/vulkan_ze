#pragma once

#include "z0/vulkan/vulkan_shader.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/vulkan/vulkan_image.hpp"
#include "z0/vulkan/vulkan_renderer.hpp"

namespace z0 {


    class BaseRenderpass {
    public:
        virtual void cleanup();

    protected:
        VkDevice device;
        VulkanDevice& vulkanDevice;
        std::string shaderDirectory;
        VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> descriptorSets{MAX_FRAMES_IN_FLIGHT};
        std::unique_ptr<VulkanDescriptorSetLayout> globalSetLayout {};
        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::shared_ptr<VulkanDescriptorPool> globalPool {};
        std::vector<std::unique_ptr<VulkanBuffer>> globalBuffers{MAX_FRAMES_IN_FLIGHT};

        const VkClearValue clearColor {{{
                    static_cast<float>(WINDOW_CLEAR_COLOR[0]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[1]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[2]) / 256.0f,
                    1.0f}}};
        const VkClearValue depthClearValue { .depthStencil = {1.0f, 0} };

        BaseRenderpass(VulkanDevice& device, std::string shaderDirectory);

        // Helpers function for children classes
        void setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);
        void createResources();
        void writeUniformBuffer(const std::vector<std::unique_ptr<VulkanBuffer>>& buffers, uint32_t currentFrame, void *data, uint32_t index = 0);
        void createUniformBuffers(std::vector<std::unique_ptr<VulkanBuffer>>& buffers, VkDeviceSize size, uint32_t count = 1);
        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count = 0, uint32_t *offsets = nullptr);
        void bindShaders(VkCommandBuffer commandBuffer);
        std::unique_ptr<VulkanShader> createShader(const std::string& filename,
                                                   VkShaderStageFlagBits stage,
                                                   VkShaderStageFlags next_stage);

    private:
        void buildShader(VulkanShader& shader);
        void createPipelineLayout();
        std::vector<char> readFile(const std::string& fileName);

        virtual void loadShaders() = 0;
        virtual void createDescriptorSetLayout() = 0;
        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;
        void bindShader(VkCommandBuffer commandBuffer, VulkanShader& shader);

    public:
        BaseRenderpass(const BaseRenderpass&) = delete;
        BaseRenderpass &operator=(const BaseRenderpass&) = delete;
        BaseRenderpass(const BaseRenderpass&&) = delete;
        BaseRenderpass &&operator=(const BaseRenderpass&&) = delete;
    };

}