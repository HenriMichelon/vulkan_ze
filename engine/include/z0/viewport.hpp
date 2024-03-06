#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"

#include "object.hpp"

namespace z0 {

    enum MSAA {
        MSAA_DISABLED   = 0,
        MSAA_2X         = 1,
        MSAA_4X         = 2,
        MSAA_8X         = 3,
        MSAA_AUTO       = 4,
    };
    class Application;

    class Viewport: public Object {
    public:
        MSAA getMSAA() const;
        void setMSAA(MSAA samples);

        void process();
        bool shouldClose() { return window.shouldClose(); }

    private:
        WindowHelper window;
        std::unique_ptr<VulkanDevice> vulkanDevice;
        std::unique_ptr<VulkanRenderer> vulkanRenderer;

        Viewport(VulkanInstance& instance, WindowMode mode, int w, int h,
                 const std::string& name, const std::string &appdir,
                 MSAA msaa);
        friend class Application;

    public:
        VulkanDevice& _getDevice() { return *vulkanDevice; }
    };

}