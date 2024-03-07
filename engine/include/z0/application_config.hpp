#pragma once

#include "z0/window.hpp"

#include <string>
#include <cstdint>

namespace z0 {
    enum MSAA {
        MSAA_DISABLED   = 0,
        MSAA_2X         = 1,
        MSAA_4X         = 2,
        MSAA_8X         = 3,
        MSAA_AUTO       = 4,
    };

    struct ApplicationConfig {
        std::string appName     = "MyApp";
        std::string appDir      = ".";
        WindowMode windowMode   = WINDOW_MODE_WINDOWED;
        uint32_t windowWidth    = 800;
        uint32_t windowHeight   = 600;
        MSAA msaa               = MSAA_DISABLED;
    };
}