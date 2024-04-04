#include "z0/vulkan/vulkan_stats.hpp"
#include "z0/application.hpp"
#include "z0/viewport.hpp"
#include "z0/log.hpp"
#include "z0/input.hpp"

#include <Jolt/RegisterTypes.h>

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

        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        temp_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        job_system = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs,
                                                                JPH::cMaxPhysicsBarriers,
                                                                JPH::thread::hardware_concurrency() - 1);
        const uint32_t cMaxBodies = 1024;
        const uint32_t cNumBodyMutexes = 0;
        const uint32_t cMaxBodyPairs = 1024;
        const uint32_t cMaxContactConstraints = 1024;
        physicsSystem.Init(cMaxBodies,
                           cNumBodyMutexes,
                           cMaxBodyPairs,
                           cMaxContactConstraints,
                           broad_phase_layer_interface,
                           object_vs_broadphase_layer_filter,
                           object_vs_object_layer_filter);
    }

    void worker(BlockingQueue<std::shared_ptr<Node>>& queue) {
        std::shared_ptr<Node> node;
        while (queue.pop(node)) {
            //std::cout << std::this_thread::get_id() << " consumed: " << node->toString() << std::endl;
            if (node->_needPhysics()) node->_physicsUpdate();
            node->onPhysicsProcess(dt);
        }
    }

    void Application::start(const std::shared_ptr<Node>& scene) {
        currentScene = scene;
        ready(currentScene);
        physicsSystem.OptimizeBroadPhase();
        viewport->loadScene(currentScene);

        // https://gafferongames.com/post/fix_your_timestep/
        using Clock = std::chrono::steady_clock;
        double t = 0.0;
        double currentTime = std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        double accumulator = 0.0;
        uint32_t frameCount = 0;
        float elapsedSeconds = 0.0;

        constexpr int maxWorkers = 2;
        std::vector<std::jthread> workers{maxWorkers};
        for (int i = 0; i < maxWorkers; i++) {
            workers.push_back(std::jthread{worker, std::ref(queue)});
        }
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
                while ((!queue.isEmpty()) && (!viewport->shouldClose())) {}; //queue.waitWhileNotEmpty();
                physicsSystem.Update(dt, 1, temp_allocator.get(), job_system.get());
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
        queue.shutdown();
        viewport->wait();
        JPH::UnregisterTypes();
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
        if (node->isProcessed()) {
            queue.push(node);
            //node->onPhysicsProcess(delta);
        }
        for(auto& child: node->getChildren()) {
            physicsProcess(child, delta);
        }
    }
}