// Using one descriptor per scene with offsets
// https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/log.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <utility>
#include <filesystem>

namespace z0 {

    VulkanRenderer::VulkanRenderer(VulkanDevice &dev, std::string sDir) :
        vulkanDevice{dev}, device(dev.getDevice()), shaderDirectory(std::move(sDir))
    {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
            .setMaxSets(MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
            .build();

        // Create command buffers
        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Command_buffers
        {
            commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            const VkCommandBufferAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = vulkanDevice.getCommandPool(),
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
            };
            if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                die("failed to allocate command buffers!");
            }
        }

        // Create sync objects
        {
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
    }

    VulkanRenderer::~VulkanRenderer() {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void VulkanRenderer::loadResources() {
        loadModels();
        // Create Pipeline Layout
        // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction
        {
            createDescriptorSetLayout();
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
        loadShaders();
    }

    void VulkanRenderer::updateFrame() {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        update(deltaTime);
    }

    void VulkanRenderer::writeUniformBuffer(void *data, uint32_t index) {
        uint32_t size = uboBuffers[currentFrame]->getAlignmentSize();
        uboBuffers[currentFrame]->writeToBuffer(data, size, size * index);
        uboBuffers[currentFrame]->flush();
    }

    void VulkanRenderer::bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t index) {
        uint32_t size = uboBuffers[currentFrame]->getAlignmentSize();
        uint32_t offset = size * index;
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                0, 1,
                                &globalDescriptorSets[currentFrame],
                                1, &offset);
    }

    void VulkanRenderer::createUniformBuffers(VkDeviceSize size, uint32_t count) {
        for (auto &uboBuffer: uboBuffers) {
            uboBuffer = std::make_unique<VulkanBuffer>(
                    vulkanDevice,
                    size,
                    count,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    vulkanDevice.getDeviceProperties().limits.minUniformBufferOffsetAlignment
            );
            uboBuffer->map();
        }
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

        // Record command buffer
        {
            vkResetCommandBuffer(commandBuffers[currentFrame], 0);
            const VkCommandBufferBeginInfo beginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = 0,
                    .pInheritanceInfo = nullptr
            };
            if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
                die("failed to begin recording command buffer!");
            }
            setInitialState(commandBuffers[currentFrame]);
            beginRendering(commandBuffers[currentFrame], imageIndex);
            recordCommands(commandBuffers[currentFrame]);
            endRendering(commandBuffers[currentFrame], imageIndex);
            if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
                die("failed to record command buffer!");
            }
        }

        const VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        {
            const VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
            const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            updateFrame();
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
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkanDevice.getWindowHelper().windowResized) {
                vulkanDevice.recreateSwapChain();
            } else if (result != VK_SUCCESS) {
                die("failed to present swap chain image!");
            }

        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    // https://github.com/KhronosGroup/Vulkan-Samples/blob/main/samples/extensions/shader_object/shader_object.cpp
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
            vkCmdSetViewportWithCount(commandBuffer, 1, &viewport);
            const VkRect2D scissor{
                .offset = {0, 0},
                .extent = extent
            };
            vkCmdSetScissorWithCount(commandBuffer, 1, &scissor);
        }

        {
            vkCmdSetRasterizerDiscardEnable(commandBuffer, VK_FALSE);
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

        std::vector<VkVertexInputBindingDescription2EXT> vertexBinding = VulkanModel::getBindingDescription();
        std::vector<VkVertexInputAttributeDescription2EXT> vertexAttribute = VulkanModel::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());

        // Set the topology to triangles, don't restart primitives
        vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdSetPrimitiveRestartEnableEXT(commandBuffer, VK_FALSE);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, vulkanDevice.getSamples());

        const VkSampleMask sample_mask = 0xffffffff;
        vkCmdSetSampleMaskEXT(commandBuffer, vulkanDevice.getSamples(), &sample_mask);

        // Do not use alpha to coverage or alpha to one because not using MSAA
        vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_TRUE);

        vkCmdSetPolygonModeEXT(commandBuffer, VK_POLYGON_MODE_FILL);

        // Set front face, cull mode is set in build_command_buffers.
        vkCmdSetFrontFace(commandBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        // Set depth state, the depth write. Don't enable depth bounds, bias, or stencil test.
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS);
        vkCmdSetDepthBoundsTestEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthBiasEnable(commandBuffer, VK_FALSE);
        vkCmdSetStencilTestEnable(commandBuffer, VK_FALSE);

        // Do not enable logic op
        vkCmdSetLogicOpEnableEXT(commandBuffer, VK_FALSE);

        // Disable color blending
        VkBool32 color_blend_enables[] = {VK_FALSE};
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);

        // Use RGBA color write mask
        VkColorComponentFlags color_component_flags[] = {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_A_BIT};
        vkCmdSetColorWriteMaskEXT(commandBuffer, 0, 1, color_component_flags);

    }

    std::unique_ptr<VulkanShader> VulkanRenderer::createShader(const std::string& filename,
                                                               VkShaderStageFlagBits stage,
                                                               VkShaderStageFlags next_stage) {
        auto code = readFile(filename);
        std::unique_ptr<VulkanShader> shader  = std::make_unique<VulkanShader>(
                vulkanDevice,
                stage,
                next_stage,
                filename,
                code,
                globalSetLayout->getDescriptorSetLayout(),
                nullptr);
        buildShader(*shader);
        return shader;
    }

    // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
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

    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void VulkanRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           vulkanDevice.getDepthImage(),
                                           vulkanDevice.getDepthFormat(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           vulkanDevice.getColorImage(),
                                           vulkanDevice.getSwapChainImageFormat(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        // Color attachement : where the rendering is done (multisampled memory image)
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = vulkanDevice.getColorImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = vulkanDevice.getDepthImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = depthClearValue,
        };
        const VkRect2D renderArea{{0, 0}, vulkanDevice.getSwapChainExtent()};
        const VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .renderArea = renderArea,
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = &depthAttachmentInfo,
            .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void VulkanRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(
                commandBuffer,
                vulkanDevice.getSwapChainImages()[imageIndex],
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vulkanDevice.presentToSwapChain(commandBuffer, imageIndex);
        vulkanDevice.transitionImageLayout(
                commandBuffer,
                vulkanDevice.getSwapChainImages()[imageIndex],
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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

}