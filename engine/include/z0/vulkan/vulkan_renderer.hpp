#pragma once

#include "z0/vulkan/vulkan_device.hpp"
#include "vulkan_shader.hpp"
#include "vulkan_model.hpp"

namespace z0 {
    class VulkanRenderer {
    public:
        explicit VulkanRenderer(VulkanDevice& device, std::string shaderDirectory);
        ~VulkanRenderer();

        void drawFrame();

    private:
        std::unique_ptr<VulkanModel> model;

        std::string shaderDirectory;
        VulkanDevice& vulkanDevice;
        VkDevice device;
        VkCommandPool commandPool;

        const int MAX_FRAMES_IN_FLIGHT = 2;
        uint32_t currentFrame = 0;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        void createCommandPool();
        void createCommandBuffers();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void createSyncObjects();
        void setInitialState(VkCommandBuffer commandBuffer);

        void beginRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex);
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex);
        void transitionImageToOptimal(VkCommandBuffer commandBuffer,uint32_t imageIndex);
        void transitionImageToPresentSrc(VkCommandBuffer commandBuffer,uint32_t imageIndex);

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