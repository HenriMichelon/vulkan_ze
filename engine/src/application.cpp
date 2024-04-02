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

        // https://gafferongames.com/post/fix_your_timestep/
        using Clock = std::chrono::steady_clock;
        double t = 0.0;
        const float dt = 0.01; // Fixed delta time
        double currentTime = std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        double accumulator = 0.0;
        uint32_t frameCount = 0;
        float elapsedSeconds = 0.0;
        while (!viewport->shouldClose()) {
            while(Input::haveInputEvent()) {
                auto event = Input::consumeInputEvent();
                input(currentScene, *event);
            }

            double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now().time_since_epoch()).count();
            double frameTime = newTime - currentTime;
            if (frameTime > 0.25) frameTime = 0.25; // Note: Max frame time to avoid spiral of death
            currentTime = newTime;
            accumulator += frameTime;
            while (accumulator >= dt) {
                physicsProcess(currentScene, dt);
                t += dt;
                accumulator -= dt;
            }
            const double alpha = accumulator / dt;
            process(currentScene, static_cast<float>(alpha));
            viewport->drawFrame();

            elapsedSeconds += static_cast<float>(frameTime);
            frameCount++;
            if (elapsedSeconds >= 0.250) {
                auto fps = static_cast<float>(frameCount) / elapsedSeconds;
#ifdef VULKAN_STATS
                VulkanStats::get().averageFps = static_cast<uint32_t >((static_cast<float>(VulkanStats::get().averageFps) + fps) / 2.0f);
#endif
                viewport->_setFPS(fps);
                frameCount = 0;
                elapsedSeconds = 0;
            }
        }
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