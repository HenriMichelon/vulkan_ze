#pragma once

#include "scene.hpp"
#include "mainloop.hpp"

namespace z0 {

    class Application {
    public:
        static Scene& getCurrentScene() { return *MainLoop::get().currentScene; }
        static Viewport& getViewport() { return *MainLoop::get().viewport; }
        static const std::filesystem::path getDirectory() { return MainLoop::get().applicationConfig.appDir; }
        static const ApplicationConfig& getConfig() { return MainLoop::get().applicationConfig; }
    };
}