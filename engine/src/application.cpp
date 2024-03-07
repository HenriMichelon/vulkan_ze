#include "z0/application.hpp"
#include "z0/viewport.hpp"

#include <chrono>

namespace z0 {

    static Application* instance = nullptr;

    Application& Application::getApp() { return *instance; }

    Application::Application(const ApplicationConfig& cfg):
            applicationConfig{cfg},
            vulkanInstance{} {
        viewport = std::make_shared<Viewport>(vulkanInstance, cfg);
        if (instance != nullptr) die("Application already registered");
        instance = this;
    }

    void Application::start(const std::shared_ptr<Node>& rootNode) {
        rootNode->onReady(); // make recursive
        viewport->loadScene(rootNode);
        while (!viewport->shouldClose()) {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            rootNode->onProcess(deltaTime);
            viewport->process(deltaTime);
        }
    }
}