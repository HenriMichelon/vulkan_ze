#pragma once

#include "z0/vulkan/renderers/base_postprocessing_renderer.hpp"
#include "z0/vulkan/framebuffers/color_attachment_hdr.hpp"
#include "z0/vulkan/framebuffers/color_attachment.hpp"
#include "z0/vulkan/framebuffers/depth_buffer.hpp"

namespace z0 {

    class TonemappingRenderer: public BasePostprocessingRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float gamma{2.2};
            alignas(4) float exposure{1.0};
        };

        TonemappingRenderer(VulkanDevice& device,
                            std::string shaderDirectory,
                            std::shared_ptr<ColorAttachmentHDR>& inputColorAttachmentHdr,
                            std::shared_ptr<DepthBuffer> depthBuffer);

        void update(uint32_t currentFrame) override;
        void loadShaders() override;
        void createDescriptorSetLayout() override;
        void recreateImagesResources();

    private:
        std::shared_ptr<DepthBuffer> resolvedDepthBuffer;

    };

}