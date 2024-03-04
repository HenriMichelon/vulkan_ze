#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/vulkan/vulkan_ubo.hpp"
#include "z0/log.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <utility>
#include <filesystem>

namespace z0 {

    VulkanRenderer::VulkanRenderer(VulkanDevice &dev, std::string sDir) :
        vulkanDevice{dev}, device(dev.getDevice()), shaderDirectory(std::move(sDir)) {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                .build();

        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorSetLayout();
        createPipelineLayout();
        createShaders();
        createTextureImage();

        const std::vector<VulkanModel::Vertex> vertices = {
                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
                0, 1, 2, 2, 3, 0
        };
        VulkanModel::Builder builder{vertices, indices};
        model = std::make_unique<VulkanModel>(vulkanDevice, builder);
    }

    VulkanRenderer::~VulkanRenderer() {
        vkDeviceWaitIdle(device);
        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);
        fragShader.reset();
        fragShader.reset();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void VulkanRenderer::update(uint32_t frameIndex) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkanDevice.getSwapChainExtent().width / (float) vulkanDevice.getSwapChainExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();
    }

    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
    void VulkanRenderer::drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device,
                              vulkanDevice.getSwapChain(),
                              UINT64_MAX,
                              imageAvailableSemaphores[currentFrame],
                              VK_NULL_HANDLE,
                              &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            vulkanDevice.recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            die("failed to acquire swap chain image!");
        }
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        const VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        {
            const VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
            const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            update(currentFrame);

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
            result = vkQueuePresentKHR(vulkanDevice.getPresentQueue(), &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkanDevice.framebufferResized) {
                vulkanDevice.recreateSwapChain();
            } else if (result != VK_SUCCESS) {
                die("failed to present swap chain image!");
            }

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

    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction
    void VulkanRenderer::createPipelineLayout() {
        const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = globalSetLayout->getDescriptorSetLayout(),
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr
        };
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            die("failed to create pipeline layout!");
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
            vkCmdSetCullModeEXT(commandBuffer, VK_CULL_MODE_NONE);
            vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_FALSE);
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout,
                                    0,
                                    1,
                                    &globalDescriptorSets[currentFrame],
                                    0,
                                    nullptr);
            bindShader(commandBuffer, *vertShader);
            bindShader(commandBuffer, *fragShader);
            VkShaderStageFlagBits geo_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            vkCmdBindShadersEXT(commandBuffer, 1, &geo_stage, nullptr);
            model->bind(commandBuffer);
            model->draw(commandBuffer);
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

        std::vector<VkVertexInputBindingDescription2EXT> vertexBinding = VulkanModel::Vertex::getBindingDescription();
        std::vector<VkVertexInputAttributeDescription2EXT> vertexAttribute = VulkanModel::Vertex::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());


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

    void VulkanRenderer::createDescriptorSetLayout() {
        for(int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<VulkanBuffer>(
                    vulkanDevice,
                    sizeof(UniformBufferObject),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
                .addBinding(0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
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
                globalSetLayout->getDescriptorSetLayout(),
                nullptr
                );
        auto fragCode = readFile("triangle.frag");
        fragShader = std::make_unique<VulkanShader>(
                vulkanDevice,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                "triangle frag",
                fragCode,
                globalSetLayout->getDescriptorSetLayout(),
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

    // TODO make a generic function or class
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

    // TODO make a generic function or class
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


    // https://vulkan-tutorial.com/Texture_mapping/Images

    VkCommandBuffer VulkanRenderer::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(vulkanDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(vulkanDevice.getGraphicsQueue());

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void VulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void VulkanRenderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        // TODO make a generic function or class
        VkImageMemoryBarrier imageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = 0, // TODO
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = oldLayout,
                .newLayout = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image,
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                },
        };

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            die("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage,
                destinationStage,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1, // imageMemoryBarrierCount
                &imageMemoryBarrier // pImageMemoryBarriers
        );

        endSingleTimeCommands(commandBuffer);
    }

    void VulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                1
        };

        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
        );

        endSingleTimeCommands(commandBuffer);
    }

    void VulkanRenderer::createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("../texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        VulkanBuffer textureStagingBuffer{
                vulkanDevice,
                imageSize,
                1,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        textureStagingBuffer.map();
        textureStagingBuffer.writeToBuffer(pixels);
        stbi_image_free(pixels);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional
        if (vkCreateImage(device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
            die("failed to create image!");
        }

        // bind : make a class
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, textureImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VulkanDevice::findMemoryType(vulkanDevice.getPhysicalDevice(),
                                                                 memRequirements.memoryTypeBits,
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, textureImage, textureImageMemory, 0);

        transitionImageLayout(textureImage,
                              VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(textureStagingBuffer.getBuffer(),
                          textureImage,
                          static_cast<uint32_t>(texWidth),
                          static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage,
                              VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    }


}