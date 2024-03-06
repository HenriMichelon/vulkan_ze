#include "z0/vulkan/default_renderer.hpp"
#include "z0/log.hpp"
#include "z0/node.hpp"
#include "z0/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>
#include <array>

namespace z0 {

    DefaultRenderer::DefaultRenderer(VulkanDevice &dev, const std::string& sDir) : VulkanRenderer{dev, sDir} {}

    DefaultRenderer::~DefaultRenderer() {
        vkDeviceWaitIdle(device);
    }

    void DefaultRenderer::loadModels() {
        texture = std::make_unique<VulkanTexture>(vulkanDevice, "../models/cube_diffuse.png");
        model = VulkanModel::createModelFromFile(vulkanDevice, "../models/cube.obj");
        texture1 = std::make_unique<VulkanTexture>(vulkanDevice, "../models/sphere_diffuse.png");
        model1 = VulkanModel::createModelFromFile(vulkanDevice, "../models/sphere.obj");
    }

    void DefaultRenderer::loadShaders() {
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void DefaultRenderer::update(float delta) {
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
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta * glm::radians(90.0f) / 2, glm::vec3(1.0f, 0.0f, 1.0f));
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
        ubo.model = trans * rot;
        ubo.textureBinding = 0;
        uint32_t size = uboBuffers[currentFrame]->getAlignmentSize();
        uint32_t offset = size * 0;
        uboBuffers[currentFrame]->writeToBuffer(&ubo, size, offset);
        uboBuffers[currentFrame]->flush();

        rot = glm::rotate(glm::mat4(1.0f), delta * glm::radians(-90.0f) / 2, glm::vec3(1.0f, 0.0f, 1.0f));
        trans = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
        ubo.model = trans * rot;
        ubo.textureBinding = 1;
        offset += size * 1;
        uboBuffers[currentFrame]->writeToBuffer(&ubo, size, offset);
        uboBuffers[currentFrame]->flush();
    }

    void DefaultRenderer::recordCommands(VkCommandBuffer commandBuffer) {
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        bindShader(commandBuffer, *vertShader);
        bindShader(commandBuffer, *fragShader);

        uint32_t size = uboBuffers[currentFrame]->getAlignmentSize();
        uint32_t offset = size * 0;
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                0, 1,
                                &globalDescriptorSets[currentFrame],
                                1, &offset);
        model->bind(commandBuffer);
        model->draw(commandBuffer);

        offset += size * 1;
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                0, 1,
                                &globalDescriptorSets[currentFrame],
                                1, &offset);
        model1->bind(commandBuffer);
        model1->draw(commandBuffer);
    }

    void DefaultRenderer::createDescriptorSetLayout() {
        // Using one descriptor per scene with offsets
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        for(auto & uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<VulkanBuffer>(
                    vulkanDevice,
                    sizeof(UniformBufferObject),
                    2,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    vulkanDevice.getDeviceProperties().limits.minUniformBufferOffsetAlignment
            );
            uboBuffer->map();
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

}