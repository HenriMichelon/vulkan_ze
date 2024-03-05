#include "z0/application.hpp"
#include "z0/vulkan/vulkan_device.hpp"
#include "z0/vulkan/vulkan_renderer.hpp"

namespace z0 {

    Application::Application(int w, int h, const std::string &name, const std::string &appdir):
            viewport{w, h, name, appdir}
    {
        while (!viewport.shouldClose()) {
            viewport.process();
        }
    }
}