#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/application.hpp"

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

    Viewport::Viewport(VulkanInstance& instance, WindowMode mode, int w, int h,
                       const std::string &name, const std::string &appdir,
                       MSAA msaa):
        window{mode, w, h, name}
    {
        vulkanDevice = std::make_unique<VulkanDevice>(
                instance,
                window,
                msaa != MSAA_DISABLED,
                MSAA_VULKAN.at(msaa));
        vulkanRenderer = std::make_unique<VulkanRenderer>(*vulkanDevice, appdir + "/shaders");
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
}