#include "z0/vulkan/renderers/scene_renderer.hpp"
#include "z0/mainloop.hpp"

namespace z0 {

    static const std::map<MSAA, VkSampleCountFlagBits> MSAA_VULKAN {
            //{ MSAA_DISABLED, VK_SAMPLE_COUNT_1_BIT },
            { MSAA_2X, VK_SAMPLE_COUNT_2_BIT },
            { MSAA_4X, VK_SAMPLE_COUNT_4_BIT },
            { MSAA_8X, VK_SAMPLE_COUNT_8_BIT },
            { MSAA_AUTO, VK_SAMPLE_COUNT_1_BIT },
    };
    static const std::map<VkSampleCountFlagBits, MSAA> VULKAN_MSAA {
            //{ VK_SAMPLE_COUNT_1_BIT, MSAA_DISABLED },
            { VK_SAMPLE_COUNT_2_BIT, MSAA_2X },
            { VK_SAMPLE_COUNT_4_BIT, MSAA_4X },
            { VK_SAMPLE_COUNT_8_BIT, MSAA_8X },
    };

    Viewport::Viewport(VulkanInstance& instance, const ApplicationConfig& cfg):
        window{cfg.windowMode, static_cast<int>(cfg.windowWidth), static_cast<int>(cfg.windowHeight), cfg.appName}
    {
        vulkanDevice = std::make_unique<VulkanDevice>(
                instance,
                window,
                cfg.msaa == MSAA_AUTO,
                MSAA_VULKAN.at(cfg.msaa));
        sceneRenderer = std::make_shared<SceneRenderer>(*vulkanDevice, (cfg.appDir / "shaders").string());
        vulkanDevice->registerRenderer(sceneRenderer);
    }

    void Viewport::wait() {
        vulkanDevice->wait();
    }

    void Viewport::drawFrame() {
        window.process();
        vulkanDevice->drawFrame();
    }

    MSAA Viewport::getMSAA() const {
        return VULKAN_MSAA.at(vulkanDevice->getSamples());
    }

    void Viewport::setMSAA(z0::MSAA samples) {
        die("not implemented");
    }

    float Viewport::getAspectRatio() const {
        return vulkanDevice->getAspectRatio();
    }

    void Viewport::loadScene(std::shared_ptr<Node>& rootNode) {
        sceneRenderer->loadScene(rootNode);
    }

#ifdef GLFW_VERSION_MAJOR
    bool Viewport::isKeyPressed(Key key) const {
        return glfwGetKey(window.getWindowHandle(), key) == GLFW_PRESS;
    }

    void Viewport::setMouseMode(MouseMode mode) const {
        int value = 0;
        switch (mode) {
            case MOUSE_MODE_VISIBLE:
                value = GLFW_CURSOR_NORMAL;
                break;
            case MOUSE_MODE_VISIBLE_CAPTURED:
                value = GLFW_CURSOR_CAPTURED;
                break;
            case MOUSE_MODE_HIDDEN:
                value = GLFW_CURSOR_HIDDEN;
                break;
            case MOUSE_MODE_HIDDEN_CAPTURED:
                value = GLFW_CURSOR_DISABLED;
                break;
        }
        glfwSetInputMode(window.getWindowHandle(), GLFW_CURSOR, value);
    }
#endif
}