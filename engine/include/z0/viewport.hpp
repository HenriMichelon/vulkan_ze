#pragma once

#include "z0/application_config.hpp"
#include "z0/object.hpp"
#include "z0/nodes/node.hpp"
#include "z0/helpers/window_helper.hpp"
#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class SceneRenderer;
    class DebugRenderer;

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

        bool haveInputEvent() const { return window.haveInputEvent(); }
        std::shared_ptr<InputEvent> consumeInputEvent() { return window.consumeEvent(); }

        void loadScene(std::shared_ptr<Node>& rootNode);

        static Viewport& get();

    private:
        float fps;
        WindowHelper window;
        std::unique_ptr<VulkanDevice> vulkanDevice;
        std::shared_ptr<SceneRenderer> sceneRenderer;

    public:
        VulkanDevice& _getDevice() { return *vulkanDevice; }
        void _setFPS(float _fps) { fps = _fps; }
    };

}