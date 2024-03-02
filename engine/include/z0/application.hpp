#pragma once

#include "z0/vulkan/window_helper.hpp"

#include <string>

namespace z0 {
    class Application {
    public:
        Application(int w, int h, const std::string& name);
    private:
        WindowHelper window;
    };
}