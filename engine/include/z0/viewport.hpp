#pragma once

#include "z0/application_config.hpp"
#include "z0/object.hpp"
#include "z0/vulkan/window_helper.hpp"
#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class Application;
    class DefaultRenderer;

    class Viewport: public Object {
    public:
        ~Viewport();

        MSAA getMSAA() const;
        void setMSAA(MSAA samples);

        void process();
        bool shouldClose() { return window.shouldClose(); }

    private:
        WindowHelper window;
        std::unique_ptr<VulkanDevice> vulkanDevice;
        std::unique_ptr<DefaultRenderer> vulkanRenderer;

        Viewport(VulkanInstance& instance, const ApplicationConfig& applicationConfig);
        friend class Application;

    public:
        VulkanDevice& _getDevice() { return *vulkanDevice; }
    };

}