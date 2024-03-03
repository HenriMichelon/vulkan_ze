#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class VulkanRenderer {
    public:
        VulkanRenderer(VulkanDevice& device);
        ~VulkanRenderer();

        void drawFrame();

    private:
        VulkanDevice& device;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;

        void createGraphicsPipeline();
        void createCommandPool();
        void createCommandBuffer();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void createSyncObjects();

        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void transitionImageToOptimal(uint32_t imageIndex);
        void transitionImageToPresentSrc(uint32_t imageIndex);

        VkShaderModule createShaderModule(const std::vector<char>& code);
        static std::vector<char> readFile(const std::string& filepath);

    public:
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer &operator=(const VulkanRenderer&) = delete;
        VulkanRenderer(const VulkanRenderer&&) = delete;
        VulkanRenderer &&operator=(const VulkanRenderer&&) = delete;
    };

}