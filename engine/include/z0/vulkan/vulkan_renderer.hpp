#pragma once

#include <volk.h>

namespace z0 {

    class VulkanRenderer {
    public:
        virtual VkImage getImage() const { return VK_NULL_HANDLE; };
        virtual VkImageView getImageView() const { return VK_NULL_HANDLE; };
        virtual void cleanup() = 0;
        virtual void update(uint32_t currentFrame) = 0;
        virtual void beginRendering(VkCommandBuffer commandBuffer) = 0;
        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;
        virtual void endRendering(VkCommandBuffer commandBuffer, bool isLast)  = 0;
        virtual void createImagesResources() = 0;
        virtual void cleanupImagesResources() = 0;
        virtual void recreateImagesResources() = 0;
    };

}