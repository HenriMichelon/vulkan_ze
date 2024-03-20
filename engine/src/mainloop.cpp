#include "z0/mainloop.hpp"
#include "z0/viewport.hpp"
#include "z0/vulkan/vulkan_stats.hpp"

#include <chrono>

namespace z0 {

    static MainLoop* instance = nullptr;

    MainLoop& MainLoop::get() { return *instance; }

    MainLoop::MainLoop(const ApplicationConfig& cfg):
            applicationConfig{cfg},
            vulkanInstance{} {
        viewport = std::make_shared<Viewport>(vulkanInstance, cfg);
        if (instance != nullptr) die("MainLoop already registered");
        instance = this;
    }

    void MainLoop::start(const std::shared_ptr<Scene>& scene) {
        if (!scene->isValid()) return;
        currentScene = scene;
        std::shared_ptr<Node>& rootNode = currentScene->getRootNode();
        ready(rootNode);
        viewport->loadScene(rootNode);

        uint32_t frameCount = 0;
        auto lastFrameTime = std::chrono::high_resolution_clock::now();
        float elapsedSeconds = 0.0;
        while (!viewport->shouldClose()) {
            auto currentFrameTime = std::chrono::high_resolution_clock::now();
            auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - lastFrameTime).count();

            process(rootNode, deltaTime);
            viewport->drawFrame();

            elapsedSeconds += deltaTime;
            frameCount++;
            if (elapsedSeconds >= 1.0) {
                viewport->_setFPS( frameCount / elapsedSeconds);
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

    void MainLoop::ready(const std::shared_ptr<Node>& node) {
        node->onReady();
        for(auto child: node->getChildren()) {
            ready(child);
        }
    }

    void MainLoop::process(const std::shared_ptr<Node>& node, float delta) {
        node->onProcess(delta);
        for(auto child: node->getChildren()) {
            process(child, delta);
        }
    }
}