#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/log.hpp"

#include <fstream>
#include <utility>
#include <filesystem>

namespace z0 {

    VulkanRenderer::VulkanRenderer(VulkanDevice &dev, std::string sDir) :
        vulkanDevice{dev}, device(dev.getDevice()), shaderDirectory(std::move(sDir)) {
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
        createShaders();
    }

    VulkanRenderer::~VulkanRenderer() {
        vkDeviceWaitIdle(device);
        fragShader.reset();
        fragShader.reset();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(device, commandPool, nullptr);
    }

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
    void VulkanRenderer::drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device,
                              vulkanDevice.getSwapChain(),
                              UINT64_MAX,
                              imageAvailableSemaphores[currentFrame],
                              VK_NULL_HANDLE,
                              &imageIndex);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        const VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        {
            const VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
            const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            const VkSubmitInfo submitInfo{
                    .sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount     = 1,
                    .pWaitSemaphores        = waitSemaphores,
                    .pWaitDstStageMask      = waitStages,
                    .commandBufferCount     = 1,
                    .pCommandBuffers        = &commandBuffers[currentFrame],
                    .signalSemaphoreCount   = 1,
                    .pSignalSemaphores      = signalSemaphores
            };
            if (vkQueueSubmit(vulkanDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                die("failed to submit draw command buffer!");
            }
        }
        {
            const VkSwapchainKHR swapChains[] = {vulkanDevice.getSwapChain()};
            const VkPresentInfoKHR presentInfo{
                    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores    = signalSemaphores,
                    .swapchainCount     = 1,
                    .pSwapchains        = swapChains,
                    .pImageIndices      = &imageIndex,
                    .pResults           = nullptr // Optional
            };
            vkQueuePresentKHR(vulkanDevice.getPresentQueue(), &presentInfo);
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanRenderer::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        const VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        const VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                die("failed to create semaphores!");
            }
        }
    }

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Command_buffers
    void VulkanRenderer::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices =
            VulkanDevice::findQueueFamilies(vulkanDevice.getPhysicalDevice(),
                                                    vulkanDevice.getSurface());
        const VkCommandPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
        };
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            die("failed to create command pool!");
        }
    }

    void VulkanRenderer::createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        const VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
        };
        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            die("failed to allocate command buffers!");
        }
    }

    void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        const VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0,
            .pInheritanceInfo = nullptr
        };
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            die("failed to begin recording command buffer!");
        }
        setInitialState(commandBuffer);
        beginRendering(commandBuffer, imageIndex);
        {
            // Disable depth write and use cull mode none to draw skybox
            vkCmdSetCullModeEXT(commandBuffer, VK_CULL_MODE_NONE);
            vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_FALSE);

            bindShader(commandBuffer, *vertShader);
            bindShader(commandBuffer, *fragShader);

            VkShaderStageFlagBits geo_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            vkCmdBindShadersEXT(commandBuffer, 1, &geo_stage, nullptr);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        }
        endRendering(commandBuffer, imageIndex);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            die("failed to record command buffer!");
        }
    }

    std::vector<char> VulkanRenderer::readFile(const std::string &fileName) {
        std::filesystem::path filepath = shaderDirectory;
        filepath /= fileName;
        filepath += ".spv";
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            die("failed to open file : ", fileName);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    void VulkanRenderer::setInitialState(VkCommandBuffer commandBuffer)
    {
        {
            const VkExtent2D extent = vulkanDevice.getSwapChainExtent();
            const VkViewport viewport{
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = static_cast<float>(extent.width),
                    .height = static_cast<float>(extent.height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
            };
            vkCmdSetViewportWithCountEXT(commandBuffer, 1, &viewport);
            const VkRect2D scissor{
                    .offset = {0, 0},
                    .extent = extent
            };
            vkCmdSetScissorWithCountEXT(commandBuffer, 1, &scissor);
        }

        {
            vkCmdSetRasterizerDiscardEnableEXT(commandBuffer, VK_FALSE);
            const VkColorBlendEquationEXT colorBlendEquation{
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
            };
            vkCmdSetColorBlendEquationEXT(commandBuffer, 0, 1, &colorBlendEquation);
        }

       /* const VkVertexInputBindingDescription2EXT vertex_binding[] =
                {
                        vkb::initializers::vertex_input_binding_description2ext(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX, 1)};

        const VkVertexInputAttributeDescription2EXT vertex_attribute_description_ext[] =
                {
                        vkb::initializers::vertex_input_attribute_description2ext(
                                0,
                                0,
                                VK_FORMAT_R32G32B32_SFLOAT,
                                offsetof(Vertex, pos)),
                        vkb::initializers::vertex_input_attribute_description2ext(
                                0,
                                1,
                                VK_FORMAT_R32G32B32_SFLOAT,
                                offsetof(Vertex, normal)),
                        vkb::initializers::vertex_input_attribute_description2ext(
                                0,
                                2,
                                VK_FORMAT_R32G32_SFLOAT,
                                offsetof(Vertex, uv)),
                };

        vkCmdSetVertexInputEXT(cmd, sizeof(vertex_binding) / sizeof(vertex_binding[0]), vertex_binding, sizeof(vertex_attribute_description_ext) / sizeof(vertex_attribute_description_ext[0]), vertex_attribute_description_ext);
        */
        // Set the topology to triangles, don't restart primitives, set samples to only 1 per pixel
        vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdSetPrimitiveRestartEnableEXT(commandBuffer, VK_FALSE);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);

        const VkSampleMask sample_mask = 0x1;
        vkCmdSetSampleMaskEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT, &sample_mask);

        // Do not use alpha to coverage or alpha to one because not using MSAA
        vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);

        vkCmdSetPolygonModeEXT(commandBuffer, VK_POLYGON_MODE_FILL);

        // Set front face, cull mode is set in build_command_buffers.
        vkCmdSetFrontFaceEXT(commandBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        // Set depth state, the depth write. Don't enable depth bounds, bias, or stencil test.
        vkCmdSetDepthTestEnableEXT(commandBuffer, VK_TRUE);
        vkCmdSetDepthCompareOpEXT(commandBuffer, VK_COMPARE_OP_GREATER);
        vkCmdSetDepthBoundsTestEnableEXT(commandBuffer, VK_FALSE);
        vkCmdSetDepthBiasEnableEXT(commandBuffer, VK_FALSE);
        vkCmdSetStencilTestEnableEXT(commandBuffer, VK_FALSE);

        // Do not enable logic op
        vkCmdSetLogicOpEnableEXT(commandBuffer, VK_FALSE);

        // Disable color blending
        VkBool32 color_blend_enables[] = {VK_FALSE};
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);

        // Use RGBA color write mask
        VkColorComponentFlags color_component_flags[] = {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_A_BIT};
        vkCmdSetColorWriteMaskEXT(commandBuffer, 0, 1, color_component_flags);
    }

    //region Shared Objects
    void VulkanRenderer::createShaders() {
        auto vertCode = readFile("triangle.vert");
        vertShader = std::make_unique<VulkanShader>(
                vulkanDevice,
                VK_SHADER_STAGE_VERTEX_BIT,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                "triangle vert",
                vertCode,
                nullptr,
                nullptr
                );
        auto fragCode = readFile("triangle.frag");
        fragShader = std::make_unique<VulkanShader>(
                vulkanDevice,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                "triangle frag",
                fragCode,
                nullptr,
                nullptr
        );
        buildLinkedShaders(*vertShader, *fragShader);
    }

    void VulkanRenderer::buildLinkedShaders(VulkanShader& vert, VulkanShader& frag)
    {
        VkShaderCreateInfoEXT shader_create_infos[2];
        shader_create_infos[0] = vert.getShaderCreateInfo();
        shader_create_infos[1] = frag.getShaderCreateInfo();
        for (auto &shader_create : shader_create_infos){
            shader_create.flags |= VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
        }
        VkShaderEXT shaderEXTs[2];
        if (vkCreateShadersEXT(
                device,
                2,
                shader_create_infos,
                nullptr,
                shaderEXTs) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed\n");
        }
        vert.setShader(shaderEXTs[0]);
        frag.setShader(shaderEXTs[1]);
    }

    void VulkanRenderer::buildShader(VulkanShader& shader) {
        VkShaderEXT shaderEXT;
        VkShaderCreateInfoEXT shaderCreateInfo = shader.getShaderCreateInfo();
        if (vkCreateShadersEXT(device, 1, &shaderCreateInfo, nullptr, &shaderEXT) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed");
        }
        shader.setShader(shaderEXT);
    }

    void VulkanRenderer::bindShader(VkCommandBuffer commandBuffer, VulkanShader& shader) {
        vkCmdBindShadersEXT(commandBuffer, 1, shader.getStage(), shader.getShader());
    }
    //endregion

    //region Dynamic Rendering
    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void VulkanRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        transitionImageToOptimal(commandBuffer, imageIndex);
        const VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        /*const VkClearValue depthClearValue{
            .depthStencil = {0.f, 0}
        };*/
        const VkRenderingAttachmentInfoKHR colorAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = vulkanDevice.getSwapChainImageViews()[imageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearColor,
        };
        /*const VkRenderingAttachmentInfoKHR depthAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = vulkanDevice.getSwapChainImageViews()[imageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = depthClearValue,
        };*/
        const VkRect2D renderArea{
            {0, 0},
            vulkanDevice.getSwapChainExtent()};
        const VkRenderingInfoKHR renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = renderArea,
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = nullptr
        };
        vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);
    }

    void VulkanRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vkCmdEndRenderingKHR(commandBuffer);
        transitionImageToPresentSrc(commandBuffer, imageIndex);
    }

    void VulkanRenderer::transitionImageToPresentSrc(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        const VkImageMemoryBarrier imageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = vulkanDevice.getSwapChainImages()[imageIndex],
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
            &imageMemoryBarrier // pImageMemoryBarriers
        );
    }

    void VulkanRenderer::transitionImageToOptimal(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        const VkImageMemoryBarrier imageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = vulkanDevice.getSwapChainImages()[imageIndex],
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
            &imageMemoryBarrier // pImageMemoryBarriers
        );
    }
    //endregion


}