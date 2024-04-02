#pragma once

#include "z0/helpers/window_helper.hpp"
#include "z0/application_config.hpp"
#include "z0/vulkan/vulkan_instance.hpp"
#include "z0/nodes/node.hpp"

namespace z0 {

    class Viewport;

    class Application: public Object {
    public:
        explicit Application(const ApplicationConfig& applicationConfig);

        void start(const std::shared_ptr<Node>& scene);

        static bool isPaused() { return get().paused; }
        static void setPaused(bool pause) { get().paused = pause; }
        static std::shared_ptr<Node>& getCurrentScene() { return get().getCurrentScene(); }
        static Viewport& getViewport() { return *get().viewport; }
        static const std::filesystem::path getDirectory() { return get().applicationConfig.appDir; }
        static const ApplicationConfig& getConfig() { return get().applicationConfig; }

    private:
        VulkanInstance vulkanInstance;
        std::shared_ptr<Viewport> viewport;
        const ApplicationConfig& applicationConfig;
        std::shared_ptr<Node> currentScene;
        bool paused{false};

        void ready(const std::shared_ptr<Node>& node);
        void process(const std::shared_ptr<Node>& node, float delta);
        void physicsProcess(const std::shared_ptr<Node>& node, float delta);
        void input(const std::shared_ptr<Node>& node, InputEvent& event);

        static Application& get();

    public:
        Application(const Application&) = delete;
        Application &operator=(const Application&) = delete;
        Application(const Application&&) = delete;
        Application &&operator=(const Application&&) = delete;
    };

}