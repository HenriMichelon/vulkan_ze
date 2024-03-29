#pragma once

#include "z0/vulkan/renderers/base_postprocessing_renderer.hpp"

namespace z0 {

    class PostprocessingRenderer: public BasePostprocessingRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float dummy;
        };

        PostprocessingRenderer(VulkanDevice& device, std::string shaderDirectory, std::shared_ptr<ColorAttachmentHDR>& inputColorAttachmentHdr);

        std::shared_ptr<ColorAttachmentHDR>& getColorAttachment() { return colorAttachmentHdr; }

        void cleanup() override;
        void update(uint32_t currentFrame) override;
        void loadShaders() override;
        void createDescriptorSetLayout()  override;
        void beginRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage, VkImageView swapChainImageView) override;
        void endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;

    private:
        std::shared_ptr<ColorAttachmentHDR> colorAttachmentHdr;
    };

}