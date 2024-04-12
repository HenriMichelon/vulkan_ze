#pragma once

#include "z0/helpers/window_helper.hpp"
#include "z0/vulkan/vulkan_device.hpp"
#include "z0/application_config.hpp"
#include "z0/nodes/node.hpp"
#include "z0/nodes/skybox.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    class SceneRenderer;
    class TonemappingRenderer;
    class SimplePostprocessingRenderer;

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

    private:
        float fps;
        WindowHelper window;
        std::unique_ptr<VulkanDevice> vulkanDevice;
        std::shared_ptr<SceneRenderer> sceneRenderer;
        std::shared_ptr<TonemappingRenderer> tonemappingRenderer;
        std::shared_ptr<SimplePostprocessingRenderer> postprocessingRenderer;

    public:
        VulkanDevice& _getDevice() { return *vulkanDevice; }
        WindowHelper& _getWindowHelper() { return window; }
        void _setFPS(float _fps) { fps = _fps; }
        void _setSkyBox(Skybox& skybox);
        void _setCamera(Camera* camera);
        Camera* _getCurrentCamera();
        void setEnvironment(Environment* environment);
        void addMesh(MeshInstance* meshInstance);
    };

}