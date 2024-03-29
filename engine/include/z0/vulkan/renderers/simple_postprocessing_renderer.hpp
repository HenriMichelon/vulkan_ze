#pragma once

#include "z0/vulkan/renderers/base_postprocessing_renderer.hpp"

namespace z0 {

    class SimplePostprocessingRenderer: public BasePostprocessingRenderer {
    public:
        struct GobalUniformBufferObject {
            alignas(4) float dummy;
        };

        SimplePostprocessingRenderer(VulkanDevice& device,
                                     std::string shaderDirectory,
                                     const std::string shaderName,
                                     std::shared_ptr<ColorAttachmentHDR>& inputColorAttachmentHdr);

        void update(uint32_t currentFrame) override;
        void loadShaders() override;
        void createDescriptorSetLayout()  override;

    private:
        const std::string shaderName;
    };

}