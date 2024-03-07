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
        ready(rootNode);
        viewport->loadScene(rootNode);
        while (!viewport->shouldClose()) {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            process(rootNode, deltaTime);
            viewport->process(deltaTime);
        }
    }

    void Application::ready(const std::shared_ptr<Node>& node) {
        node->onReady();
        for(auto child: node->getChildren()) {
            ready(child);
        }
    }

    void Application::process(const std::shared_ptr<Node>& node, float delta) {
        node->onProcess(delta);
        for(auto child: node->getChildren()) {
            process(child, delta);
        }
    }
}