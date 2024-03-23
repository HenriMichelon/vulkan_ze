#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/log.hpp"

#include <fstream>
#include <filesystem>

namespace z0 {

    BaseRenderer::BaseRenderer(VulkanDevice &dev, std::string sDir) :
        vulkanDevice{dev}, device(dev.getDevice()), shaderDirectory(std::move(sDir))
    {
    }

    void BaseRenderer::cleanup() {

        globalBuffers.clear();
        if (vertShader != nullptr) vertShader.reset();
        if (fragShader != nullptr) fragShader.reset();
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
        globalSetLayout.reset();
        globalPool.reset();
    }

    void BaseRenderer::bindShaders(VkCommandBuffer commandBuffer) {
        if (vertShader != nullptr) {
            vkCmdBindShadersEXT(commandBuffer, 1, vertShader->getStage(), vertShader->getShader());
        } else {
            VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_VERTEX_BIT};
            vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
        }
        if (fragShader != nullptr) {
            vkCmdBindShadersEXT(commandBuffer, 1, fragShader->getStage(), fragShader->getShader());
        } else {
            VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_FRAGMENT_BIT};
            vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
        }
    }

    void BaseRenderer::writeUniformBuffer(const std::vector<std::unique_ptr<VulkanBuffer>>& buffers, uint32_t currentFrame, void *data, uint32_t index) {
        uint32_t size = buffers[currentFrame]->getAlignmentSize();
        buffers[currentFrame]->writeToBuffer(data, size, size * index);
    }

    void BaseRenderer::createUniformBuffers(std::vector<std::unique_ptr<VulkanBuffer>>& buffers, VkDeviceSize size, uint32_t count) {
        for (auto &uboBuffer: buffers) {
            uboBuffer = std::make_unique<VulkanBuffer>(
                    vulkanDevice,
                    size,
                    count,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    vulkanDevice.getDeviceProperties().limits.minUniformBufferOffsetAlignment
            );
            uboBuffer->map();
        }
    }

    void BaseRenderer::bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count, uint32_t *offsets) {
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0, 1,
                                &descriptorSets[currentFrame],
                                count, offsets);
    }

    void BaseRenderer::createResources() {
        createDescriptorSetLayout();
        if (globalSetLayout != nullptr) {
            createPipelineLayout();
            loadShaders();
        }
    };

    void BaseRenderer::createPipelineLayout() {
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

    std::unique_ptr<VulkanShader> BaseRenderer::createShader(const std::string& filename,
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
    void BaseRenderer::buildShader(VulkanShader& shader) {
        VkShaderEXT shaderEXT;
        VkShaderCreateInfoEXT shaderCreateInfo = shader.getShaderCreateInfo();
        if (vkCreateShadersEXT(device, 1, &shaderCreateInfo, nullptr, &shaderEXT) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed");
        }
        shader.setShader(shaderEXT);
    }

    void BaseRenderer::setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height) {
        const VkExtent2D extent = { width, height };
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

    std::vector<char> BaseRenderer::readFile(const std::string &fileName) {
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