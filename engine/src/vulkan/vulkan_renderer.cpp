#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/vulkan/vulkan_ubo.hpp"
#include "z0/log.hpp"
#include "z0/node.hpp"
#include "z0/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <utility>
#include <filesystem>
#include <array>

namespace z0 {

    VulkanRenderer::VulkanRenderer(VulkanDevice &dev, std::string sDir) :
        vulkanDevice{dev}, device(dev.getDevice()), shaderDirectory(std::move(sDir)) {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                .build();

        texture = std::make_unique<VulkanTexture>(vulkanDevice, "../models/cube_diffuse.png");
        model = VulkanModel::createModelFromFile(vulkanDevice, "../models/cube.obj");
        texture1 = std::make_unique<VulkanTexture>(vulkanDevice, "../models/sphere_diffuse.png");
        model1 = VulkanModel::createModelFromFile(vulkanDevice, "../models/sphere.obj");

        createCommandBuffers();
        createSyncObjects();
        createDescriptorSetLayout();
        createPipelineLayout();
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
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void VulkanRenderer::update(uint32_t frameIndex) {

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
    void VulkanRenderer::createCommandBuffers() {
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
            vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
            bindShader(commandBuffer, *vertShader);
            bindShader(commandBuffer, *fragShader);

            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            Camera camera{};
            auto cameraNode = Node::create();
            cameraNode.transform.translation.z = -5.0f;
            cameraNode.transform.translation.y = 0.0f;
            cameraNode.transform.rotation.x = 0.0f;
            camera.setViewYXZ(cameraNode.transform.translation, cameraNode.transform.rotation);
            float aspect = vulkanDevice.getSwapChainAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

            UniformBufferObject ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f) / 2, glm::vec3(1.0f, 0.0f, 1.0f));
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
            ubo.model = trans * rot;
            ubo.textureBinding = 0;
            uint32_t size = uboBuffers[currentFrame]->getAlignmentSize();
            uint32_t offset = size * 0;
            uboBuffers[currentFrame]->writeToBuffer(&ubo, size, offset);
            uboBuffers[currentFrame]->flush();

            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout,
                                    0,
                                    1,
                                    &globalDescriptorSets[currentFrame],
                                    1,
                                    &offset);
            model->bind(commandBuffer);
            model->draw(commandBuffer);

            rot = glm::rotate(glm::mat4(1.0f), time * glm::radians(-90.0f) / 2, glm::vec3(1.0f, 0.0f, 1.0f));
            trans = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
            ubo.model = trans * rot;
            ubo.textureBinding = 1;
            offset += size * 1;
            uboBuffers[currentFrame]->writeToBuffer(&ubo, size, offset);
            uboBuffers[currentFrame]->flush();
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout,
                                    0,
                                    1,
                                    &globalDescriptorSets[currentFrame],
                                    1,
                                    &offset);
            model1->bind(commandBuffer);
            model1->draw(commandBuffer);
        }
        endRendering(commandBuffer, imageIndex);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            die("failed to record command buffer!");
        }
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

    void VulkanRenderer::createDescriptorSetLayout() {
        // Using one descriptor per scene with offsets
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        for(int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<VulkanBuffer>(
                    vulkanDevice,
                    sizeof(UniformBufferObject),
                    2,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    vulkanDevice.getDeviceProperties().limits.minUniformBufferOffsetAlignment
            );
            uboBuffers[i]->map();
        }
        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
                .addBinding(0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                            VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_ALL_GRAPHICS,
                            2)
                .build();
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo(sizeof(UniformBufferObject));
            std::array imagesInfo{ texture->imageInfo(), texture1->imageInfo()};
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, imagesInfo.data())
                    .build(globalDescriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

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

    //region Dynamic Rendering
    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void VulkanRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vulkanDevice.transitionImageLayout(
                commandBuffer,
                vulkanDevice.getSwapChainImages()[imageIndex],
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
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
        // Color attachement : where the rendering is done
        const VkClearValue clearColor = {{{0.5f, 0.5f, 0.5f, 1.0f}}};
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = vulkanDevice.getColorImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        // made it instance wide ?
        const VkClearValue depthClearValue{ .depthStencil = {1.0f, 0} };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = vulkanDevice.getDepthImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = depthClearValue,
        };
        std::array<VkRenderingAttachmentInfo, 1> renderingAttachmentInfo = { colorAttachmentInfo };
        const VkRect2D renderArea{
            {0, 0},
            vulkanDevice.getSwapChainExtent()};
        const VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .renderArea = renderArea,
            .layerCount = 1,
            .colorAttachmentCount = renderingAttachmentInfo.size(),
            .pColorAttachments = renderingAttachmentInfo.data(),
            .pDepthAttachment = &depthAttachmentInfo,
            .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void VulkanRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.presentToSwapChain(commandBuffer, imageIndex);
        vulkanDevice.transitionImageLayout(
                commandBuffer,
                vulkanDevice.getSwapChainImages()[imageIndex],
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    //endregion

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