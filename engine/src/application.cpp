#include "z0/application.hpp"

namespace z0 {

    static Application* instance = nullptr;

    Application& Application::getApp() { return *instance; }

    Application::Application(WindowMode mode, int w, int h, const std::string &name, std::string _appdir):
            appdir(_appdir), vulkanInstance{}, viewport{vulkanInstance, mode, w, h, name, _appdir}
    {
        if (instance != nullptr) die("Application already registered");
        instance = this;
        while (!viewport.shouldClose()) {
            viewport.process();
        }
    }
}