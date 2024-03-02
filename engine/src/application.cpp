#include "z0/application.hpp"
#include "z0/vulkan/main_loop.hpp"

namespace z0 {
    Application::Application(int w, int h, const std::string &name) {
        Z0MainLoop mainLoop{*this, w, h, name};
    }
}