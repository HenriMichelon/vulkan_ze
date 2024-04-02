#include "z0/vulkan/vulkan_stats.hpp"
#include "z0/application.hpp"
#include "z0/viewport.hpp"
#include "z0/log.hpp"
#include "z0/input.hpp"

#include <chrono>

namespace z0 {

    static Application* instance = nullptr;

    Application& Application::get() { return *instance; }

    Application::Application(const ApplicationConfig& cfg):
            applicationConfig{cfg},
            vulkanInstance{} {
        viewport = std::make_shared<Viewport>(vulkanInstance, cfg);
        if (instance != nullptr) die("Application already registered");
        instance = this;
    }

    void Application::start(const std::shared_ptr<Node>& scene) {
        currentScene = scene;
        ready(currentScene);
        viewport->loadScene(currentScene);

        uint32_t frameCount = 0;
        auto lastFrameTime = std::chrono::high_resolution_clock::now();
        float elapsedSeconds = 0.0;
        while (!viewport->shouldClose()) {
            while(Input::haveInputEvent()) {
                auto event = Input::consumeInputEvent();
                input(currentScene, *event);
            }

            auto currentFrameTime = std::chrono::high_resolution_clock::now();
            auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - lastFrameTime).count();

            process(currentScene, deltaTime);
            viewport->drawFrame();

            elapsedSeconds += deltaTime;
            frameCount++;
            if (elapsedSeconds >= 0.250) {
                auto fps = frameCount / elapsedSeconds;
#ifdef VULKAN_STATS
                VulkanStats::get().averageFps = (VulkanStats::get().averageFps + fps) / 2;
#endif
                viewport->_setFPS( fps);
                frameCount = 0;
                elapsedSeconds = 0;
            }
            lastFrameTime = currentFrameTime;
        }
        viewport->wait();
#ifdef VULKAN_STATS
        VulkanStats::get().display();
#endif
    }

    void Application::input(const std::shared_ptr<Node>& node, InputEvent& event) {
        if (node->isProcessed()) node->onInput(event);
        for(auto child: node->getChildren()) {
            input(child, event);
        }
    }

    void Application::ready(const std::shared_ptr<Node>& node) {
        node->onReady();
        for(auto child: node->getChildren()) {
            ready(child);
        }
    }

    void Application::process(const std::shared_ptr<Node>& node, float delta) {
        if (node->isProcessed()) node->onProcess(delta);
        for(auto child: node->getChildren()) {
            process(child, delta);
        }
    }
}