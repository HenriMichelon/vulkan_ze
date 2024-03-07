#include "z0/vulkan/default_renderer.hpp"
#include "z0/application.hpp"
#include "z0/mesh.hpp"
#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    static const std::map<MSAA, VkSampleCountFlagBits> MSAA_VULKAN {
            { MSAA_DISABLED, VK_SAMPLE_COUNT_1_BIT },
            { MSAA_2X, VK_SAMPLE_COUNT_2_BIT },
            { MSAA_4X, VK_SAMPLE_COUNT_4_BIT },
            { MSAA_8X, VK_SAMPLE_COUNT_8_BIT },
            { MSAA_AUTO, VK_SAMPLE_COUNT_1_BIT },
    };
    static const std::map<VkSampleCountFlagBits, MSAA> VULKAN_MSAA {
            { VK_SAMPLE_COUNT_1_BIT, MSAA_DISABLED },
            { VK_SAMPLE_COUNT_2_BIT, MSAA_2X },
            { VK_SAMPLE_COUNT_4_BIT, MSAA_4X },
            { VK_SAMPLE_COUNT_8_BIT, MSAA_8X },
    };

    Viewport::Viewport(VulkanInstance& instance, const ApplicationConfig& cfg):
        window{cfg.windowMode, cfg.windowWidth, cfg.windowHeight, cfg.appName}
    {
        vulkanDevice = std::make_unique<VulkanDevice>(
                instance,
                window,
                cfg.msaa != MSAA_DISABLED,
                MSAA_VULKAN.at(cfg.msaa));
        vulkanRenderer = std::make_unique<DefaultRenderer>(*vulkanDevice, cfg.appDir + "/shaders");
    }

    Viewport::~Viewport() {
    }

    void Viewport::process() {
        window.process();
        vulkanRenderer->drawFrame();
    }

    MSAA Viewport::getMSAA() const {
        return VULKAN_MSAA.at(vulkanDevice->getSamples());
    }

    void Viewport::setMSAA(z0::MSAA samples) {
        die("not implemented");
    }

    void Viewport::loadScene(const std::shared_ptr<Node>& rootNode) {
        vulkanRenderer->loadScene(rootNode);

    }
}