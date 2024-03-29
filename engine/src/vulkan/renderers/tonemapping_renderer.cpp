#include "z0/vulkan/renderers/tonemapping_renderer.hpp"
#include "z0/application.hpp"

namespace z0 {

    TonemappingRenderer::TonemappingRenderer(VulkanDevice &dev,
                                             std::string shaderDirectory,
                                             std::shared_ptr<ColorAttachmentHDR>& _inputColorAttachmentHdr):
            BasePostprocessingRenderer{dev, shaderDirectory, _inputColorAttachmentHdr} {
        createResources();
    }

    void TonemappingRenderer::loadShaders() {
        BasePostprocessingRenderer::loadShaders();
        fragShader = createShader("reinhard.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void TonemappingRenderer::update(uint32_t currentFrame) {
        GobalUniformBufferObject globalUbo {
            .gamma = Application::getConfig().gamma,
            .exposure = Application::getConfig().exposure,
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);
    }

    void TonemappingRenderer::createDescriptorSetLayout() {
        BasePostprocessingRenderer::createDescriptorSetLayout(sizeof(GobalUniformBufferObject));
    }

}