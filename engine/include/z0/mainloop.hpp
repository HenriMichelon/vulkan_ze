#pragma once

#include "z0/vulkan/window_helper.hpp"
#include "z0/nodes/node.hpp"
#include "z0/application_config.hpp"
#include "z0/vulkan/vulkan_instance.hpp"

namespace z0 {

    class Viewport;

    class MainLoop: public Object {
    public:
        MainLoop(const ApplicationConfig& applicationConfig);

        void start(const std::shared_ptr<Node>& rootNode);

        static MainLoop& get();

    private:
        VulkanInstance vulkanInstance;
        std::shared_ptr<Viewport> viewport;
        const ApplicationConfig& applicationConfig;

        void ready(const std::shared_ptr<Node>& node);
        void process(const std::shared_ptr<Node>& node, float delta);

        friend class Application;

    public:
        MainLoop(const MainLoop&) = delete;
        MainLoop &operator=(const MainLoop&) = delete;
        MainLoop(const MainLoop&&) = delete;
        MainLoop &&operator=(const MainLoop&&) = delete;
    };

    class Application {
    public:
        static Viewport& getViewport() { return *MainLoop::get().viewport; }
        static const std::filesystem::path getDirectory() { return MainLoop::get().applicationConfig.appDir; }
    };


}