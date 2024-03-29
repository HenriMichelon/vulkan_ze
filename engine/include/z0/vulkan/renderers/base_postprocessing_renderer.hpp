#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/framebuffers/color_attachment_hdr.hpp"

namespace z0 {

    class BasePostprocessingRenderer: public BaseRenderer, public VulkanRenderer {
    public:
        BasePostprocessingRenderer(VulkanDevice& device, std::string shaderDirectory, std::shared_ptr<ColorAttachmentHDR>& inputColorAttachmentHdr);

        void loadShaders() override;
        void createDescriptorSetLayout(VkDeviceSize globalUboSize) ;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

    private:
        std::shared_ptr<ColorAttachmentHDR> inputColorAttachmentHdr;
    };

}