#pragma once

#include "z0/vulkan/window_helper.hpp"

namespace z0 {

    class VulkanDevice;

    class DebugUI {
    public:
        DebugUI(VulkanDevice& vulkanDevice, WindowHelper& windowHelper);

        void drawFrame(VkCommandBuffer commandBuffer, VkImageView targetImageView, VkExtent2D extent);
        void cleanup(VulkanDevice& device);

        void updateFPS(float fps, float deltaTime);

    private:
        WindowHelper& windowHelper;
        VkDescriptorPool imguiPool;
        float fps;
        float deltaTime;

        const ImGuiWindowFlags transparentWindowFlags =
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize;

        void defaultStyle();
        void drawUI();

        void quitButton();
        void statsPanel();
    };
}