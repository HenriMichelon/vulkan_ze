#pragma once

#include "z0/vulkan/vulkan_device.hpp"
#include "z0/vulkan/vulkan_shader.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/vulkan/vulkan_image.hpp"

namespace z0 {

    const int MAX_FRAMES_IN_FLIGHT = 2;

    class VulkanRenderer {
    public:
        ~VulkanRenderer();

        void drawFrame(float delta);

    protected:
        uint32_t currentFrame = 0;
        VkDevice device;
        VulkanDevice& vulkanDevice;
        VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };
        std::unique_ptr<VulkanDescriptorPool> globalPool {};
        std::vector<std::unique_ptr<VulkanBuffer>> surfacesBuffers{MAX_FRAMES_IN_FLIGHT};
        std::unique_ptr<VulkanDescriptorSetLayout> surfacesSetLayout {};
        std::vector<VkDescriptorSet> surfacesDescriptorSets{MAX_FRAMES_IN_FLIGHT};

        VulkanRenderer(VulkanDevice& device, std::string shaderDirectory);

        // Helpers function for children classes
        void createResources();
        void createUniformBuffers(std::vector<std::unique_ptr<VulkanBuffer>>& buffers, VkDeviceSize size = VK_WHOLE_SIZE, uint32_t count = 1);
        void writeUniformBuffer(const std::vector<std::unique_ptr<VulkanBuffer>>& buffers, void *data, uint32_t index = 1);
        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t index);
        void bindShader(VkCommandBuffer commandBuffer, VulkanShader& shader);
        std::unique_ptr<VulkanShader> createShader(const std::string& filename,
                                                   VkShaderStageFlagBits stage,
                                                   VkShaderStageFlags next_stage);
        float getAspectRatio() const {
            return static_cast<float>(vulkanDevice.getSwapChainExtent().width) / static_cast<float>(vulkanDevice.getSwapChainExtent().height);
        }

    private:
        std::string shaderDirectory;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        const VkClearValue clearColor {{{
          static_cast<float>(WINDOW_CLEAR_COLOR[0]) / 256.0f,
          static_cast<float>(WINDOW_CLEAR_COLOR[1]) / 256.0f,
          static_cast<float>(WINDOW_CLEAR_COLOR[2]) / 256.0f,
          1.0f}}};
        const VkClearValue depthClearValue { .depthStencil = {1.0f, 0} };

        void setInitialState(VkCommandBuffer commandBuffer);
        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex);
        void buildShader(VulkanShader& shader);
        void createPipelineLayout();
        std::vector<char> readFile(const std::string& fileName);

        virtual void update(float delta) = 0;
        virtual void recordCommands(VkCommandBuffer commandBuffer) = 0;
        virtual void createDescriptorSetLayout() = 0;
        virtual void loadShaders() = 0;

    public:
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer &operator=(const VulkanRenderer&) = delete;
        VulkanRenderer(const VulkanRenderer&&) = delete;
        VulkanRenderer &&operator=(const VulkanRenderer&&) = delete;
    };

}