#pragma once

#include "z0/vulkan/vulkan_device.hpp"
#include "z0/vulkan/vulkan_shader.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/vulkan/vulkan_texture.hpp"

namespace z0 {
    class VulkanRenderer {
    public:
        explicit VulkanRenderer(VulkanDevice& device, std::string shaderDirectory);
        ~VulkanRenderer();

        void drawFrame();

    private:
        std::string shaderDirectory;
        VulkanDevice& vulkanDevice;
        VkDevice device;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<VulkanDescriptorSetLayout> globalSetLayout;
        std::unique_ptr<VulkanDescriptorPool> globalPool{};

        std::vector<VkDescriptorSet> globalDescriptorSets{2}; // MAX_FRAMES_IN_FLIGHT
        std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers{2}; // MAX_FRAMES_IN_FLIGHT

        const int MAX_FRAMES_IN_FLIGHT = 2;
        uint32_t currentFrame = 0;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        std::unique_ptr<VulkanModel> model;
        std::unique_ptr<VulkanTexture> texture;
        std::unique_ptr<VulkanModel> model1;
        std::unique_ptr<VulkanTexture> texture1;

        void createPipelineLayout();
        void createCommandBuffers();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void createSyncObjects();
        void setInitialState(VkCommandBuffer commandBuffer);
        void createDescriptorSetLayout();
        void update(uint32_t frameIndex);

        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex);

        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        void createShaders();
        void buildShader(VulkanShader& shader);
        void buildLinkedShaders(VulkanShader& vert, VulkanShader& frag);
        void bindShader(VkCommandBuffer commandBuffer,VulkanShader& shader);

        std::vector<char> readFile(const std::string& fileName);

    public:
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer &operator=(const VulkanRenderer&) = delete;
        VulkanRenderer(const VulkanRenderer&&) = delete;
        VulkanRenderer &&operator=(const VulkanRenderer&&) = delete;
    };

}