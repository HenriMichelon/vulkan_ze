#pragma once

#include "z0/vulkan/window_helper.hpp"
#include "z0/viewport.hpp"

namespace z0 {

    class Application: public Object {
    public:
        Application(const ApplicationConfig& applicationConfig);

        const std::string& getApplicationDirectory() const { return appdir; }

        static Application& getApp();

    private:
        VulkanInstance vulkanInstance;
        Viewport viewport;
        const std::string appdir;

    public:
        Application(const Application&) = delete;
        Application &operator=(const Application&) = delete;
        Application(const Application&&) = delete;
        Application &&operator=(const Application&&) = delete;
    };

}