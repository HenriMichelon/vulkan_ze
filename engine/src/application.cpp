#include "z0/application.hpp"
#include "z0/vulkan/window_helper.hpp"

namespace z0 {
    Application::Application(int w, int h, const std::string &name): window{w, h, name} {
        while (!window.shouldClose()) {
            window.process();
        }
    }
}