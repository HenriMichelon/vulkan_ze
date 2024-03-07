#pragma once

#include "z0/application_config.hpp"
#include "z0/object.hpp"
#include "z0/nodes/node.hpp"
#include "z0/vulkan/window_helper.hpp"
#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class DefaultRenderer;

    class Viewport: public Object {
    public:
        Viewport(VulkanInstance& instance, const ApplicationConfig& applicationConfig);
        ~Viewport();

        MSAA getMSAA() const;
        void setMSAA(MSAA samples);

        void process(float delta);
        bool shouldClose() { return window.shouldClose(); }

        void loadScene(const std::shared_ptr<Node>& rootNode);

        static Viewport& get();

    private:
        WindowHelper window;
        std::unique_ptr<VulkanDevice> vulkanDevice;
        std::unique_ptr<DefaultRenderer> vulkanRenderer;


    public:
        VulkanDevice& _getDevice() { return *vulkanDevice; }
    };

}