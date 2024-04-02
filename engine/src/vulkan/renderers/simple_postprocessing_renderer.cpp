#include "z0/vulkan/renderers/simple_postprocessing_renderer.hpp"

namespace z0 {

    SimplePostprocessingRenderer::SimplePostprocessingRenderer(VulkanDevice &dev,
                                                               std::string shaderDirectory,
                                                               const std::string _shaderName,
                                                               std::shared_ptr<ColorAttachmentHDR>& inputColorAttachmentHdr):
            BasePostprocessingRenderer{dev, shaderDirectory, inputColorAttachmentHdr}, shaderName{_shaderName} {
        createResources();
    }

    void SimplePostprocessingRenderer::loadShaders() {
        BasePostprocessingRenderer::loadShaders();
        fragShader = createShader(shaderName + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SimplePostprocessingRenderer::update(uint32_t currentFrame) {
        GobalUniformBufferObject globalUbo {
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);
    }

    void SimplePostprocessingRenderer::createDescriptorSetLayout() {
        BasePostprocessingRenderer::createGlobalDescriptorSetLayout(sizeof(GobalUniformBufferObject));
    }

}