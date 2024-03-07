#pragma once

#include "z0/vulkan/window_helper.hpp"
#include "z0/nodes/node.hpp"
#include "z0/application_config.hpp"
#include "z0/vulkan/vulkan_instance.hpp"

namespace z0 {

    class Viewport;

    class Application: public Object {
    public:
        Application(const ApplicationConfig& applicationConfig);

        void start(const std::shared_ptr<Node>& rootNode);

        static Viewport& getViewport() { return *getApp().viewport; }
        static const std::string& getDirectory() { return getApp().applicationConfig.appDir; }
        static Application& getApp();

    private:
        VulkanInstance vulkanInstance;
        std::shared_ptr<Viewport> viewport;
        const ApplicationConfig& applicationConfig;

        void ready(const std::shared_ptr<Node>& node);
        void process(const std::shared_ptr<Node>& node, float delta);

    public:
        Application(const Application&) = delete;
        Application &operator=(const Application&) = delete;
        Application(const Application&&) = delete;
        Application &&operator=(const Application&&) = delete;
    };

}