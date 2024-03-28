#include <array>
#include "z0/vulkan/renderers/tonemapping_renderer.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/log.hpp"

namespace z0 {

    TonemappingRenderer::TonemappingRenderer(VulkanDevice &dev, std::string shaderDirectory, ToneMap& _toneMap):
    BaseRenderer{dev, shaderDirectory}, toneMap{toneMap} {
    }

    void TonemappingRenderer::cleanup() {
        BaseRenderer::cleanup();
    }

    void TonemappingRenderer::loadShaders() {
        vertShader = createShader("quad.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("tonemaps/reinhard.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
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
            auto imageInfo = toneMap.imageInfo();
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &globalBufferInfo)
                    .writeImage(1, &imageInfo);
            if (!writer.build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

}