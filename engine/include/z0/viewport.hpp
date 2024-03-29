#pragma once

#include "z0/application_config.hpp"
#include "z0/nodes/node.hpp"
#include "z0/helpers/window_helper.hpp"
#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class SceneRenderer;
    class TonemappingRenderer;

    class Viewport: public Object {
    public:
        Viewport(VulkanInstance& instance, const ApplicationConfig& applicationConfig);

        MSAA getMSAA() const;
        void setMSAA(MSAA samples);
        float getAspectRatio() const;

        void drawFrame();
        void wait();
        bool shouldClose() { return window.shouldClose(); }
        float getFPS() const { return fps; }

        void loadScene(std::shared_ptr<Node>& rootNode);

        static Viewport& get();

    private:
        float fps;
        WindowHelper window;
        std::unique_ptr<VulkanDevice> vulkanDevice;
        std::shared_ptr<SceneRenderer> sceneRenderer;
        std::shared_ptr<TonemappingRenderer> tonemappingRenderer;

    public:
        VulkanDevice& _getDevice() { return *vulkanDevice; }
        WindowHelper& _getWindowHelper() { return window; }
        void _setFPS(float _fps) { fps = _fps; }
    };

}