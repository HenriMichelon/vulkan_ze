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
        while (!viewport->shouldClose()) {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            process(rootNode, deltaTime);
            viewport->drawFrame();
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