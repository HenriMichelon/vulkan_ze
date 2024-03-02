#include "z0/application.hpp"
#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {
    Application::Application(int w, int h, const std::string &name): window{w, h, name} {
        VulkanDevice vulkanDevice{window};
        while (!window.shouldClose()) {
            window.process();
        }
    }
}