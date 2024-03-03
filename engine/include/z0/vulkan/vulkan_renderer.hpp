#pragma once

#include "z0/vulkan/vulkan_device.hpp"
#include "vulkan_shader.hpp"

namespace z0 {

    class VulkanRenderer {
    public:
        explicit VulkanRenderer(VulkanDevice& device);
        ~VulkanRenderer();

        void drawFrame();

    private:
        VulkanDevice& vulkanDevice;
        VkDevice device;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;

        void createCommandPool();
        void createCommandBuffer();
        void recordCommandBuffer(uint32_t imageIndex);
        void createSyncObjects();
        void setInitialState();

        void beginRendering(uint32_t imageIndex);
        void endRendering(uint32_t imageIndex);
        void transitionImageToOptimal(uint32_t imageIndex);
        void transitionImageToPresentSrc(uint32_t imageIndex);

        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        void createShaders();
        void buildShader(VulkanShader *shader);
        void buildLinkedShaders( VulkanShader *vert, VulkanShader *frag);
        void bindShader( VulkanShader *shader);

        static std::vector<char> readFile(const std::string& filepath);

    public:
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer &operator=(const VulkanRenderer&) = delete;
        VulkanRenderer(const VulkanRenderer&&) = delete;
        VulkanRenderer &&operator=(const VulkanRenderer&&) = delete;
    };

}