#include <array>
#include "z0/vulkan/renderers/tonemapping_renderer.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/log.hpp"

namespace z0 {

    TonemappingRenderer::TonemappingRenderer(VulkanDevice &dev, std::string shaderDirectory):
    BaseRenderer{dev, shaderDirectory}, colorAttachement{dev} {
        createImagesResources();
        createResources();
    }

    void TonemappingRenderer::cleanup() {
        cleanupImagesResources();
        BaseRenderer::cleanup();
    }

    void TonemappingRenderer::loadShaders() {
        vertShader = createShader("quad.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("reinhard.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void TonemappingRenderer::update(uint32_t currentFrame) {
        GobalUniformBufferObject globalUbo{
            .gamma = 2.2f
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);
    }

    void TonemappingRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        bindShaders(commandBuffer);
        // quad renderer
        vkCmdSetVertexInputEXT(commandBuffer, 0, nullptr, 0, nullptr);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        bindDescriptorSets(commandBuffer, currentFrame);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

    void TonemappingRenderer::createDescriptorSetLayout() {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                .build();
        createUniformBuffers(globalBuffers, sizeof(GobalUniformBufferObject));
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
                .addBinding(0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT,
                            1)
            .build();
        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GobalUniformBufferObject));
            auto imageInfo = toneMap->imageInfo();
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &globalBufferInfo)
                    .writeImage(1, &imageInfo);
            if (!writer.build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void TonemappingRenderer::createImagesResources() {
        toneMap = std::make_shared<ToneMap>(vulkanDevice);
        // For bliting image to swapchain
        colorImageBlit.srcOffsets[0] = {0, 0, 0 };
        colorImageBlit.srcOffsets[1] = {
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().width),
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().height), 1 };
        colorImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.srcSubresource.mipLevel = 0;
        colorImageBlit.srcSubresource.baseArrayLayer = 0;
        colorImageBlit.srcSubresource.layerCount = 1;
        colorImageBlit.dstOffsets[0] = {0, 0, 0 };
        colorImageBlit.dstOffsets[1] = {
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().width),
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().height), 1 };
        colorImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.dstSubresource.mipLevel = 0;
        colorImageBlit.dstSubresource.baseArrayLayer = 0;
        colorImageBlit.dstSubresource.layerCount = 1;
    }

    void TonemappingRenderer::cleanupImagesResources() {
        toneMap->cleanupImagesResources();
        colorAttachement.cleanupImagesResources();
    }

    void TonemappingRenderer::recreateImagesResources() {
        toneMap->cleanupImagesResources();
        toneMap->createImagesResources();
        colorAttachement.cleanupImagesResources();
        colorAttachement.createImagesResources();
    }

    void TonemappingRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        vulkanDevice.transitionImageLayout(commandBuffer, colorAttachement.getImage(),
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorAttachement.getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE ,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
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
        vulkanDevice.transitionImageLayout(commandBuffer, colorAttachement.getImage(),
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        vulkanDevice.transitionImageLayout(
                commandBuffer,swapChainImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
        // Blit image to swap chain to change format VK_FORMAT_R16G16B16A16_SFLOAT -> VK_FORMAT_B8G8R8A8_SRGB
        // https://www.reddit.com/r/vulkan/comments/e0hsth/whats_the_best_way_to_copy_between_images_with/
        vkCmdBlitImage(commandBuffer,
                       colorAttachement.getImage(),
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       swapChainImage,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1,
                       &colorImageBlit,
                       VK_FILTER_LINEAR );
        vulkanDevice.transitionImageLayout(
                commandBuffer,swapChainImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
    }


}