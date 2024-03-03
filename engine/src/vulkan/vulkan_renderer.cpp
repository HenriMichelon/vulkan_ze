#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/vulkan_pipeline_config.hpp"
#include "z0/log.hpp"

#include <fstream>

namespace z0 {

    VulkanRenderer::VulkanRenderer(VulkanDevice &dev) : device{dev} {
        createGraphicsPipeline();
        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    }

    VulkanRenderer::~VulkanRenderer() {
        vkDeviceWaitIdle(device.getDevice());
        vkDestroySemaphore(device.getDevice(), imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(device.getDevice(), renderFinishedSemaphore, nullptr);
        vkDestroyFence(device.getDevice(), inFlightFence, nullptr);
        vkDestroyCommandPool(device.getDevice(), commandPool, nullptr);
        vkDestroyPipeline(device.getDevice(), graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
    }

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
    void VulkanRenderer::drawFrame() {
        vkWaitForFences(device.getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device.getDevice(), 1, &inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device.getDevice(),
                              device.getSwapChain(),
                              UINT64_MAX,
                              imageAvailableSemaphore,
                              VK_NULL_HANDLE,
                              &imageIndex);

        vkResetCommandBuffer(commandBuffer, 0);
        recordCommandBuffer(commandBuffer, imageIndex);

        const VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        const VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        const VkSubmitInfo submitInfo{
            .sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount     = 1,
            .pWaitSemaphores        = waitSemaphores,
            .pWaitDstStageMask      = waitStages,
            .commandBufferCount     = 1,
            .pCommandBuffers        = &commandBuffer,
            .signalSemaphoreCount   = 1,
            .pSignalSemaphores      = signalSemaphores
        };
        if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            die("failed to submit draw command buffer!");
        }

        const VkSwapchainKHR swapChains[] = {device.getSwapChain()};
        const VkPresentInfoKHR presentInfo{
                .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores    = signalSemaphores,
                .swapchainCount     = 1,
                .pSwapchains        = swapChains,
                .pImageIndices      = &imageIndex,
                .pResults           = nullptr // Optional
        };
        vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);
    }

    void VulkanRenderer::createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            die("failed to create semaphores!");
        }
    }

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Command_buffers
    void VulkanRenderer::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = VulkanDevice::findQueueFamilies(device.getPhysicalDevice(),
                                                                                device.getSurface());

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        if (vkCreateCommandPool(device.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            die("failed to create command pool!");
        }
    }

    void VulkanRenderer::createCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
            die("failed to allocate command buffers!");
        }
    }

    void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            die("failed to begin recording command buffer!");
        }

        beginRendering(commandBuffer, imageIndex);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>( device.getSwapChainExtent().width);
        viewport.height = static_cast<float>( device.getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = device.getSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        endRendering(commandBuffer, imageIndex);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction
    void VulkanRenderer::createGraphicsPipeline() {
        auto vertShaderCode = readFile("shaders/triangle.vert.spv");
        auto fragShaderCode = readFile("shaders/triangle.frag.spv");
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            die("failed to create pipeline layout!");
        }

        // https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html
        VkFormat color_rendering_format = device.getSwapChainImageFormat();
        VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        pipelineRenderingCreateInfo.pNext = VK_NULL_HANDLE;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &color_rendering_format;
        //pipelineRenderingCreateInfo.depthAttachmentFormat   = depth_format;
        //pipelineRenderingCreateInfo.stencilAttachmentFormat = depth_format;

        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
        VulkanPipelineConfig defaultPipelineConfig{};
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &pipelineRenderingCreateInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &defaultPipelineConfig.vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &defaultPipelineConfig.inputAssemblyInfo;
        pipelineInfo.pViewportState = &defaultPipelineConfig.viewportInfo;
        pipelineInfo.pRasterizationState = &defaultPipelineConfig.rasterizationInfo;
        pipelineInfo.pMultisampleState = &defaultPipelineConfig.multisampleInfo;
        pipelineInfo.pDepthStencilState = &defaultPipelineConfig.depthStencilInfo;
        pipelineInfo.pColorBlendState = &defaultPipelineConfig.colorBlendInfo;
        pipelineInfo.pDynamicState = &defaultPipelineConfig.dynamicStateInfo;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                      &graphicsPipeline) != VK_SUCCESS) {
            die("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device.getDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(device.getDevice(), vertShaderModule, nullptr);
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
    VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            die("failed to create shader module!");
        }
        return shaderModule;
    }

    std::vector<char> VulkanRenderer::readFile(const std::string &filepath) {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            die("failed to open file : " + filepath);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void VulkanRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        transitionImageToOptimal(imageIndex);
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        const VkRenderingAttachmentInfoKHR color_attachment_info{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = device.getSwapChainImageViews()[imageIndex],
                .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRect2D renderArea{
                {0, 0},
                device.getSwapChainExtent()};
        const VkRenderingInfoKHR renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .renderArea = renderArea,
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_attachment_info,
        };
        device.vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);
    }

    void VulkanRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        device.vkCmdEndRenderingKHR(commandBuffer);
        transitionImageToPresentSrc(imageIndex);
    }

    void VulkanRenderer::transitionImageToPresentSrc(uint32_t imageIndex) {
        const VkImageMemoryBarrier image_memory_barrier_after{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .image = device.getSwapChainImages()[imageIndex],
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                }
        };
        vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
                0,
                0,
                nullptr,
                0,
                nullptr,
                1, // imageMemoryBarrierCount
                &image_memory_barrier_after // pImageMemoryBarriers
        );
    }

    void VulkanRenderer::transitionImageToOptimal(uint32_t imageIndex) {
        const VkImageMemoryBarrier image_memory_barrier_before{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .image = device.getSwapChainImages()[imageIndex],
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                }
        };
        vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
                0,
                0,
                nullptr,
                0,
                nullptr,
                1, // imageMemoryBarrierCount
                &image_memory_barrier_before // pImageMemoryBarriers
        );
    }


}