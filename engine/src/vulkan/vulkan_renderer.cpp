#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/vulkan_pipeline_config.hpp"
#include "z0/log.hpp"

#include <fstream>

namespace z0 {

    VulkanRenderer::VulkanRenderer(VulkanDevice &dev) : vulkanDevice{dev}, device(dev.getDevice()) {
        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
        createShaders();
    }

    VulkanRenderer::~VulkanRenderer() {
        vkDeviceWaitIdle(device);
        fragShader.reset();
        fragShader.reset();
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroyFence(device, inFlightFence, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);
    }

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
    void VulkanRenderer::drawFrame() {
        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device,
                              vulkanDevice.getSwapChain(),
                              UINT64_MAX,
                              imageAvailableSemaphore,
                              VK_NULL_HANDLE,
                              &imageIndex);

        vkResetCommandBuffer(commandBuffer, 0);
        recordCommandBuffer(imageIndex);

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
        if (vkQueueSubmit(vulkanDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            die("failed to submit draw command buffer!");
        }

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

    void VulkanRenderer::createSyncObjects() {
        const VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        const VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            die("failed to create semaphores!");
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

    void VulkanRenderer::createCommandBuffer() {
        const VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            die("failed to allocate command buffers!");
        }
    }

    void VulkanRenderer::recordCommandBuffer(uint32_t imageIndex) {
        const VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0,
            .pInheritanceInfo = nullptr
        };
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            die("failed to begin recording command buffer!");
        }
        setInitialState();
        beginRendering(imageIndex);

        {
            // Disable depth write and use cull mode none to draw skybox
            vkCmdSetCullModeEXT(commandBuffer, VK_CULL_MODE_NONE);
            vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_FALSE);

            // Bind shaders for the skybox
            bindShader(vertShader.get());
            bindShader(fragShader.get());

            VkShaderStageFlagBits geo_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            vkCmdBindShadersEXT(commandBuffer, 1, &geo_stage, nullptr);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        }

        endRendering(imageIndex);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            die("failed to record command buffer!");
        }
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction
    /*void VulkanRenderer::createGraphicsPipeline() {
        auto vertShaderCode = readFile("shaders/triangle.vert.spv");
        auto fragShaderCode = readFile("shaders/triangle.frag.spv");
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        const VkPipelineShaderStageCreateInfo vertShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = "main"
        };
        const VkPipelineShaderStageCreateInfo fragShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = "main"
        };
        const VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertShaderStageInfo,
            fragShaderStageInfo
        };
        const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
        };
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            die("failed to create pipeline layout!");
        }

        // https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html
        const VkFormat colorRenderingFormat = vulkanDevice.getSwapChainImageFormat();
        const VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = VK_NULL_HANDLE,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorRenderingFormat,
            //.depthAttachmentFormat   = depth_format,
            //.stencilAttachmentFormat = depth_format
        };

        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
        const VulkanPipelineConfig defaultPipelineConfig{};
        const VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &defaultPipelineConfig.vertexInputInfo,
            .pInputAssemblyState = &defaultPipelineConfig.inputAssemblyInfo,
            .pViewportState = &defaultPipelineConfig.viewportInfo,
            .pRasterizationState = &defaultPipelineConfig.rasterizationInfo,
            .pMultisampleState = &defaultPipelineConfig.multisampleInfo,
            .pDepthStencilState = &defaultPipelineConfig.depthStencilInfo,
            .pColorBlendState = &defaultPipelineConfig.colorBlendInfo,
            .pDynamicState = &defaultPipelineConfig.dynamicStateInfo,
            .layout = pipelineLayout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                      &graphicsPipeline) != VK_SUCCESS) {
            die("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
    VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code) {
        const VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t *>(code.data())
        };
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            die("failed to create shader module!");
        }
        return shaderModule;
    }*/

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

    void VulkanRenderer::setInitialState()
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

        // Rasterization is always enabled
        vkCmdSetRasterizerDiscardEnableEXT(commandBuffer, VK_FALSE);

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
        auto vertCode = readFile("shaders/triangle.vert.spv");
        vertShader = std::make_unique<VulkanShader>(
                vulkanDevice,
                VK_SHADER_STAGE_VERTEX_BIT,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                "triangle vert",
                vertCode,
                nullptr,
                nullptr
                );
        auto fragCode = readFile("shaders/triangle.frag.spv");
        fragShader = std::make_unique<VulkanShader>(
                vulkanDevice,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                "triangle frag",
                fragCode,
                nullptr,
                nullptr
        );
        buildLinkedShaders(vertShader.get(), fragShader.get());
    }

    void VulkanRenderer::buildLinkedShaders(VulkanShader *vert, VulkanShader *frag)
    {
        VkShaderCreateInfoEXT shader_create_infos[2];
        if (vert == nullptr || frag == nullptr) {
            die("buildLinkedShaders failed with null vertex or fragment shader");
        }

        shader_create_infos[0] = vert->getShaderCreateInfo();
        shader_create_infos[1] = frag->getShaderCreateInfo();
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
        vert->setShader(shaderEXTs[0]);
        frag->setShader(shaderEXTs[1]);
    }

    void VulkanRenderer::buildShader(VulkanShader *shader) {
        VkShaderEXT shaderEXT;
        VkShaderCreateInfoEXT shaderCreateInfo = shader->getShaderCreateInfo();
        if (vkCreateShadersEXT(device, 1, &shaderCreateInfo, nullptr, &shaderEXT) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed");
        }
        shader->setShader(shaderEXT);
    }

    void VulkanRenderer::bindShader(VulkanShader *shader) {
        vkCmdBindShadersEXT(commandBuffer, 1, shader->getStage(), shader->getShader());
    }
    //endregion

    //region Dynamic Rendering
    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void VulkanRenderer::beginRendering(uint32_t imageIndex) {
        transitionImageToOptimal(imageIndex);
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

    void VulkanRenderer::endRendering(uint32_t imageIndex) {
        vkCmdEndRenderingKHR(commandBuffer);
        transitionImageToPresentSrc(imageIndex);
    }

    void VulkanRenderer::transitionImageToPresentSrc(uint32_t imageIndex) {
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

    void VulkanRenderer::transitionImageToOptimal(uint32_t imageIndex) {
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