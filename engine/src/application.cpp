#include "z0/application.hpp"
#include "z0/vulkan/vulkan_device.hpp"
#include "z0/vulkan/vulkan_renderer.hpp"

namespace z0 {
    Application::Application(int w, int h, const std::string &name): window{w, h, name} {
        VulkanDevice vulkanDevice{window};
        VulkanRenderer vulkanRenderer{vulkanDevice, "../shaders"};
        while (!window.shouldClose()) {
            window.process();
            vulkanRenderer.drawFrame();
        }
    }
}