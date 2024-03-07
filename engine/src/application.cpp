#include "z0/application.hpp"

namespace z0 {

    static Application* instance = nullptr;

    Application& Application::getApp() { return *instance; }

    Application::Application(const ApplicationConfig& cfg):
            appdir(cfg.appDir),
            vulkanInstance{},
            viewport{vulkanInstance, cfg}
    {
        if (instance != nullptr) die("Application already registered");
        instance = this;
        while (!viewport.shouldClose()) {
            viewport.process();
        }
    }
}