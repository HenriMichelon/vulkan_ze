#include <array>
#include "z0/vulkan/renderers/tonemapping_renderer.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/log.hpp"
#include "z0/application_config.hpp"
#include "z0/application.hpp"

namespace z0 {

    TonemappingRenderer::TonemappingRenderer(VulkanDevice &dev, std::string shaderDirectory, std::shared_ptr<ColorAttachmentHDR> _inputColorAttachmentHdr):
            BasePostprocessingRenderer{dev, shaderDirectory, _inputColorAttachmentHdr}  {
        createImagesResources();
        createResources();
    }

    void TonemappingRenderer::cleanup() {
        cleanupImagesResources();
        BaseRenderer::cleanup();
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

    void TonemappingRenderer::createImagesResources() {
    }

    void TonemappingRenderer::cleanupImagesResources() {
    }

    void TonemappingRenderer::recreateImagesResources() {
    }

    void TonemappingRenderer::beginRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage, VkImageView swapChainImageView) {
        vulkanDevice.transitionImageLayout(commandBuffer, swapChainImage,
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = swapChainImageView,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE ,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, vulkanDevice.getSwapChainExtent()},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = nullptr,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void TonemappingRenderer::endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(
                commandBuffer,swapChainImage,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
    }


}