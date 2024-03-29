#include "z0/vulkan/renderers/scene_renderer.hpp"
#include "z0/vulkan/renderers/tonemapping_renderer.hpp"
#include "z0/vulkan/renderers/postprocessing_renderer.hpp"
#include "z0/mainloop.hpp"
#include "z0/log.hpp"

namespace z0 {

    static const std::map<MSAA, VkSampleCountFlagBits> MSAA_VULKAN {
            { MSAA_2X, VK_SAMPLE_COUNT_2_BIT },
            { MSAA_4X, VK_SAMPLE_COUNT_4_BIT },
            { MSAA_8X, VK_SAMPLE_COUNT_8_BIT },
            { MSAA_AUTO, VK_SAMPLE_COUNT_1_BIT },
    };
    static const std::map<VkSampleCountFlagBits, MSAA> VULKAN_MSAA {
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
        const std::string sDir{(cfg.appDir / "shaders").string()};
        sceneRenderer = std::make_shared<SceneRenderer>(*vulkanDevice, sDir);
        postprocessingRenderer = std::make_shared<PostprocessingRenderer>(*vulkanDevice, sDir, sceneRenderer->getColorAttachment());
        tonemappingRenderer = std::make_shared<TonemappingRenderer>(*vulkanDevice, sDir, postprocessingRenderer->getColorAttachment());
        vulkanDevice->registerRenderer(sceneRenderer);
        vulkanDevice->registerRenderer(postprocessingRenderer);
        vulkanDevice->registerRenderer(tonemappingRenderer);
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

}