#pragma once

#include "z0/vulkan/window_helper.hpp"

namespace z0 {

    class VulkanDevice;

    class DebugUI {
    public:
        DebugUI(VulkanDevice& vulkanDevice, WindowHelper& windowHelper);

        void drawFrame(VkCommandBuffer commandBuffer, VkImageView targetImageView, VkExtent2D extent);
        void cleanup(VulkanDevice& device);

    private:
        WindowHelper& windowHelper;
        VkDescriptorPool imguiPool;

        const ImGuiWindowFlags invisibleWindowFlags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoScrollbar ;

        void defaultStyle();
        void drawUI();

        void quitButton();
        void statsPanel();
    };
}