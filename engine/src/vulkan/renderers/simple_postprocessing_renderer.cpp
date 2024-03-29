#include <array>
#include "z0/vulkan/renderers/simple_postprocessing_renderer.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/log.hpp"

namespace z0 {

    SimplePostprocessingRenderer::SimplePostprocessingRenderer(VulkanDevice &dev,
                                                               std::string shaderDirectory,
                                                               const std::string _shaderName,
                                                               std::shared_ptr<ColorAttachmentHDR>& colorAttachmentHdr):
            BasePostprocessingRenderer{dev, shaderDirectory, colorAttachmentHdr}, shaderName{_shaderName} {
        createImagesResources();
        createResources();
    }

    void SimplePostprocessingRenderer::cleanup() {
        cleanupImagesResources();
        BaseRenderer::cleanup();
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
        BasePostprocessingRenderer::createDescriptorSetLayout(sizeof(GobalUniformBufferObject));
    }

    void SimplePostprocessingRenderer::createImagesResources() {
        colorAttachmentHdr = std::make_shared<ColorAttachmentHDR>(vulkanDevice);
    }

    void SimplePostprocessingRenderer::cleanupImagesResources() {
        colorAttachmentHdr->cleanupImagesResources();
    }

    void SimplePostprocessingRenderer::recreateImagesResources() {
        colorAttachmentHdr->cleanupImagesResources();
        colorAttachmentHdr->createImagesResources();
    }

    void SimplePostprocessingRenderer::beginRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage, VkImageView swapChainImageView) {
        vulkanDevice.transitionImageLayout(commandBuffer, colorAttachmentHdr->getImage(),
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorAttachmentHdr->getImageView(),
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

    void SimplePostprocessingRenderer::endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(commandBuffer, colorAttachmentHdr->getImage(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_ACCESS_SHADER_READ_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
    }


}