#include "z0/vulkan/default_renderer.hpp"
#include "z0/log.hpp"
#include "z0/nodes/node.hpp"
#include "z0/nodes/camera.hpp"

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
        camera.transform.position = { -0.0f, 0.0f, -5.0f };
        camera.setViewYXZ();
        camera.setPerspectiveProjection(glm::radians(50.0f), getAspectRatio(), 0.1f, 100.0f);

        UniformBufferObject ubo{
            .projection = camera.getProjection(),
            .view = camera.getView(),
            .inverseView = camera.getInverseView(),
        };

        int index = 0;
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta * glm::radians(90.0f) / 2, glm::vec3(1.0f, 0.0f, 1.0f));
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
        ubo.model = trans * rot;
        ubo.textureBinding = 0;
        writeUniformBuffer(&ubo, index);

        rot = glm::rotate(glm::mat4(1.0f), delta * glm::radians(-90.0f) / 2, glm::vec3(1.0f, 0.0f, 1.0f));
        trans = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
        ubo.model = trans * rot;
        ubo.textureBinding = 1;
        writeUniformBuffer(&ubo, ++index);
    }

    void DefaultRenderer::recordCommands(VkCommandBuffer commandBuffer) {
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        bindShader(commandBuffer, *vertShader);
        bindShader(commandBuffer, *fragShader);

        uint32_t index = 0;
        bindDescriptorSets(commandBuffer, index);
        model->draw(commandBuffer);

        bindDescriptorSets(commandBuffer, ++index);
        model1->draw(commandBuffer);
    }

    void DefaultRenderer::createDescriptorSetLayout() {
        VkDeviceSize size = sizeof(UniformBufferObject);
        createUniformBuffers(size, 2);
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
            auto bufferInfo = uboBuffers[i]->descriptorInfo(size);
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