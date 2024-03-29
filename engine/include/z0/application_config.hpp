#pragma once

#include "z0/window.hpp"

#include <string>
#include <cstdint>
#include <filesystem>

namespace z0 {
    enum MSAA {
        MSAA_AUTO       = 0,
        MSAA_2X         = 1,
        MSAA_4X         = 2,
        MSAA_8X         = 3,
    };

    struct ApplicationConfig {
        std::string appName             = "MyApp";
        std::filesystem::path appDir    = ".";
        WindowMode windowMode           = WINDOW_MODE_WINDOWED;
        uint32_t windowWidth            = 800;
        uint32_t windowHeight           = 600;
        MSAA msaa                       = MSAA_2X;
        float gamma                     = 1.0f;
        float exposure                  = 1.0f;
    };
}