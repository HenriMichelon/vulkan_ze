#pragma once

#include "z0/vulkan/vulkan_device.hpp"
#include "z0/vulkan/vulkan_shader.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/vulkan/vulkan_texture.hpp"

namespace z0 {

    const int MAX_FRAMES_IN_FLIGHT = 2;

    class VulkanRenderer {
    public:
        explicit VulkanRenderer(VulkanDevice& device, std::string shaderDirectory);
        ~VulkanRenderer();

        void loadResources();
        void drawFrame();

    protected:
        uint32_t currentFrame = 0;
        VkDevice device;
        VulkanDevice& vulkanDevice;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<VulkanDescriptorPool> globalPool {};
        std::unique_ptr<VulkanDescriptorSetLayout> globalSetLayout {};
        std::vector<VkDescriptorSet> globalDescriptorSets{MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers{MAX_FRAMES_IN_FLIGHT};

        void buildShader(VulkanShader& shader);
        void bindShader(VkCommandBuffer commandBuffer, VulkanShader& shader);
        std::shared_ptr<VulkanShader> createShader(const std::string& filename,
                                                   VkShaderStageFlagBits stage,
                                                   VkShaderStageFlags next_stage);

    private:
        std::string shaderDirectory;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<std::shared_ptr<VulkanShader>> shaders {};

        void createCommandBuffers();
        void createSyncObjects();
        void createPipelineLayout();
        void setInitialState(VkCommandBuffer commandBuffer);
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex);
        void updateFrame();
        //void buildLinkedShaders(VulkanShader& vert, VulkanShader& frag);
        std::vector<char> readFile(const std::string& fileName);

        virtual void update(float delta) = 0;
        virtual void recordCommands(VkCommandBuffer commandBuffer) = 0;
        virtual void createDescriptorSetLayout() = 0;
        virtual void loadModels() = 0;
        virtual void loadShaders() = 0;

    public:
        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer &operator=(const VulkanRenderer&) = delete;
        VulkanRenderer(const VulkanRenderer&&) = delete;
        VulkanRenderer &&operator=(const VulkanRenderer&&) = delete;
    };

}