#include "z0/vulkan/vulkan_stats.hpp"
#include "z0/application.hpp"
#include "z0/viewport.hpp"
#include "z0/log.hpp"
#include "z0/input.hpp"

#include <chrono>
#include <thread>

namespace z0 {

    static Application* instance = nullptr;

    Application& Application::get() { return *instance; }

    Application::Application(const ApplicationConfig& cfg):
            vulkanInstance{}, applicationConfig{cfg} {
        viewport = std::make_shared<Viewport>(vulkanInstance, cfg);
        if (instance != nullptr) die("Application already registered");
        instance = this;
    }

    void Application::start(const std::shared_ptr<Node>& scene) {
        currentScene = scene;
        ready(currentScene);
        viewport->loadScene(currentScene);

        const std::chrono::milliseconds TIME_PER_UPDATE(16); // Fixed update rate of approx. 60 times per second
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds accumulator(0);
        auto lastRenderTime = std::chrono::steady_clock::now();
        uint32_t frameCount = 0;
        float elapsedSeconds = 0.0;
        while (!viewport->shouldClose()) {
            while(Input::haveInputEvent()) {
                auto event = Input::consumeInputEvent();
                input(currentScene, *event);
            }

            // Time management
            auto newTime = std::chrono::steady_clock::now();
            auto frameTime = duration_cast<std::chrono::milliseconds>(newTime - currentTime);
            currentTime = newTime;
            accumulator += frameTime;
            // Fixed update loop
            while (accumulator >= TIME_PER_UPDATE) {
                // Fixed update logic (e.g., physics)
                physicsProcess(currentScene, TIME_PER_UPDATE.count() / 1000.0f);
                accumulator -= TIME_PER_UPDATE;
            }
            // Variable update (rendering and less sensitive logic)
            auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - lastRenderTime).count();
            //auto deltaTime = duration_cast<std::chrono::milliseconds>(newTime - lastRenderTime);
            lastRenderTime = newTime;

            // Variable update logic (e.g., rendering, animations)
            process(currentScene, deltaTime);
            viewport->drawFrame();

            // Sleep to prevent the loop from running too fast
            //std::this_thread::sleep_until(currentTime + TIME_PER_UPDATE - accumulator);

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
        }

        /*uint32_t frameCount = 0;
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
        }*/
        viewport->wait();
#ifdef VULKAN_STATS
        VulkanStats::get().display();
#endif
    }

    void Application::input(const std::shared_ptr<Node>& node, InputEvent& event) {
        if (node->isProcessed()) node->onInput(event);
        for(auto& child: node->getChildren()) {
            input(child, event);
        }
    }

    void Application::ready(const std::shared_ptr<Node>& node) {
        for(auto& child: node->getChildren()) {
            ready(child);
        }
        node->_onReady();
    }

    void Application::process(const std::shared_ptr<Node>& node, float delta) {
        if (node->isProcessed()) node->onProcess(delta);
        for(auto& child: node->getChildren()) {
            process(child, delta);
        }
    }
    void Application::physicsProcess(const std::shared_ptr<Node>& node, float delta) {
        if (node->isProcessed()) node->onPhysicsProcess(delta);
        for(auto& child: node->getChildren()) {
            physicsProcess(child, delta);
        }
    }
}