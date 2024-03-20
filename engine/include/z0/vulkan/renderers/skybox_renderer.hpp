#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/vulkan_cubemap.hpp"

namespace z0 {

    class SkyboxRenderer: public BaseRenderer {
    public:
        SkyboxRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        void loadScene(std::shared_ptr<VulkanCubemap>& cubemap);

    private:
        std::shared_ptr<VulkanCubemap> cubemap;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;
        std::unique_ptr<VulkanBuffer> vertexBuffer;
        uint32_t vertexCount;

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) override;

    };

}