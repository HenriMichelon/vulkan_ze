#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class VulkanRenderer {
    public:
        explicit VulkanRenderer(VulkanDevice& device);
        ~VulkanRenderer();

        void drawFrame();

    private:
        VulkanDevice& vulkanDevice;
        VkDevice device;
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
        void recordCommandBuffer(uint32_t imageIndex);
        void createSyncObjects();

        void beginRendering(uint32_t imageIndex);
        void endRendering(uint32_t imageIndex);
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