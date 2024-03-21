#pragma once

#include "z0/helpers/window_helper.hpp"
#include "z0/application_config.hpp"
#include "z0/vulkan/vulkan_instance.hpp"
#include "z0/scene.hpp"

namespace z0 {

    class Viewport;

    class MainLoop: public Object {
    public:
        explicit MainLoop(const ApplicationConfig& applicationConfig);

        void start(const std::shared_ptr<Scene>& scene);

        static MainLoop& get();

    private:
        VulkanInstance vulkanInstance;
        std::shared_ptr<Viewport> viewport;
        const ApplicationConfig& applicationConfig;
        std::shared_ptr<Scene> currentScene;

        void ready(const std::shared_ptr<Node>& node);
        void process(const std::shared_ptr<Node>& node, float delta);
        void input(const std::shared_ptr<Node>& node, InputEvent& event);

        friend class Application;

    public:
        MainLoop(const MainLoop&) = delete;
        MainLoop &operator=(const MainLoop&) = delete;
        MainLoop(const MainLoop&&) = delete;
        MainLoop &&operator=(const MainLoop&&) = delete;
    };

}