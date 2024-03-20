#include "z0/vulkan/renderers/skybox_renderer.hpp"
#include "z0/log.hpp"

namespace z0 {

    SkyboxRenderer::SkyboxRenderer(VulkanDevice &dev, const std::string& sDir) : BaseRenderer{dev, sDir}{
        float skyboxVertices[] = {
                // positions
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f
        };
        vertexCount = 108 / 3;
        uint32_t  vertexSize = sizeof(float) * 3;
        VkDeviceSize bufferSize = vertexSize * vertexCount;
        VulkanBuffer stagingBuffer {
                vulkanDevice,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)skyboxVertices);
        vertexBuffer = std::make_unique<VulkanBuffer>(
                vulkanDevice,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        stagingBuffer.copyTo(*vertexBuffer, bufferSize);
    }

    void SkyboxRenderer::loadScene(std::shared_ptr<VulkanCubemap>& _cubemap) {
        cubemap = _cubemap;
        createResources();
    }

    void SkyboxRenderer::loadShaders() {
        vertShader = createShader("skybox.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("skybox.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SkyboxRenderer::update(uint32_t currentFrame) {
    }

    void SkyboxRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        bindShaders(commandBuffer);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE);
        setViewport(commandBuffer, vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height);
        VkVertexInputBindingDescription2EXT bindingDescription {
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
            .binding = 0,
            .stride = 3 * sizeof(float),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            .divisor = 1,
        };
        VkVertexInputAttributeDescription2EXT attributeDescription {
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                0
        };
        vkCmdSetVertexInputEXT(commandBuffer,
                               1,
                               &bindingDescription,
                               1,
                               &attributeDescription);
        bindDescriptorSets(commandBuffer, currentFrame);
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    void SkyboxRenderer::createDescriptorSetLayout() {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                .build();
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        1)
            .build();
        for (auto & descriptorSet : descriptorSets) {
            auto imageInfo = cubemap->imageInfo();
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeImage(0, &imageInfo);
            if (!writer.build(descriptorSet)) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void SkyboxRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        vulkanDevice.transitionImageLayout(
            commandBuffer,
            image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);

        const VkRenderingAttachmentInfo colorAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = imageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearColor,
        };
        const VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .renderArea = {{0, 0},
                           {vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height}},
            .layerCount = 1,
            .viewMask = 0b00111111,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr,
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void SkyboxRenderer::endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(
                commandBuffer,swapChainImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
        vulkanDevice.transitionImageLayout(
                commandBuffer,
                image,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);

    }

    void SkyboxRenderer::createImagesResources() {
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width,
                                 vulkanDevice.getSwapChainExtent().height,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 vulkanDevice.getSwapChainImageFormat(),
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory,
                                 VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                                 6);
        vulkanDevice.createImageView(image,
                                     vulkanDevice.getSwapChainImageFormat(),
                                     VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void SkyboxRenderer::cleanupImagesResources() {
        vkDestroyImageView(device, imageView, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, imageMemory, nullptr);
    }

    void SkyboxRenderer::recreateImagesResources() {
        cleanupImagesResources();
        createImagesResources();
    }

}