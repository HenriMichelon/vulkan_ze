#include "z0/vulkan/renderers/skybox_renderer.hpp"
#include "z0/log.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"

namespace z0 {

    SkyboxRenderer::SkyboxRenderer(VulkanDevice &dev, std::string shaderDirectory): BaseRenderer{dev, shaderDirectory} {
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

    void SkyboxRenderer::cleanup() {
        vertexBuffer.reset();
        cubemap.reset();
        BaseRenderer::cleanup();
    }

    void SkyboxRenderer::loadScene(std::shared_ptr<VulkanCubemap>& _cubemap, Camera* camera) {
        currentCamera = camera;
        cubemap = _cubemap;
        createResources();
    }

    void SkyboxRenderer::loadShaders() {
        vertShader = createShader("skybox.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("skybox.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SkyboxRenderer::update(uint32_t currentFrame) {
        GobalUniformBufferObject globalUbo{
                .projection = currentCamera->getProjection(),
                .view = currentCamera->getView(),
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);
    }

    void SkyboxRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        bindShaders(commandBuffer);
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE);
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
            auto imageInfo = cubemap->imageInfo();
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &globalBufferInfo)
                    .writeImage(1, &imageInfo);
            if (!writer.build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

}