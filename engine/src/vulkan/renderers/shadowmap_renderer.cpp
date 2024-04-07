/*
 * https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
 * https://github.com/SaschaWillems/Vulkan/tree/master/examples/shadowmapping
 */
#include "z0/vulkan/renderers/shadowmap_renderer.hpp"
#include "z0/nodes/spot_light.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/log.hpp"

#include <array>

namespace z0 {

    ShadowMapRenderer::ShadowMapRenderer(VulkanDevice &dev,
                                         const std::string& sDir) : BaseRenderpass{dev, sDir} {}

    void ShadowMapRenderer::cleanup() {
        cleanupImagesResources();
        shadowMap.reset();
        modelsBuffers.clear();
        BaseRenderpass::cleanup();
    }

    void ShadowMapRenderer::loadScene(std::shared_ptr<ShadowMap>& _shadowMap, std::vector<MeshInstance*>& _meshes) {
        meshes = _meshes;
        shadowMap = _shadowMap;
        createResources();
    }

    void ShadowMapRenderer::loadShaders() {
        vertShader = createShader("shadowmap.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
    }

    void ShadowMapRenderer::update(uint32_t currentFrame) {
        GlobalUniform globalUbo {
            .lightSpace = shadowMap->getLightSpace()
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);

        auto modelsUboArray = std::make_unique<ModelUniform[]>(meshes.size());
        uint32_t modelIndex = 0;
        for (const auto&meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                modelsUboArray[modelIndex] = {
                        .matrix = meshInstance->getTransformGlobal(),
                };
            }
            modelIndex += 1;
        }
    }

    void ShadowMapRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        bindShaders(commandBuffer);

        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthBiasEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthBias(commandBuffer, depthBiasConstant, 0.0f, depthBiasSlope);
        setViewport(commandBuffer, shadowMap->size, shadowMap->size);

        std::vector<VkVertexInputBindingDescription2EXT> vertexBinding = VulkanModel::getBindingDescription();
        std::vector<VkVertexInputAttributeDescription2EXT> vertexAttribute = VulkanModel::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());

        uint32_t modelIndex = 0;
        for (const auto&meshInstance: meshes) {
            auto mesh = meshInstance->getMesh();
            if (mesh->isValid()) {
                for (const auto& surface: mesh->getSurfaces()) {
                    if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                        vkCmdSetCullMode(commandBuffer,
                                         standardMaterial->cullMode == CULLMODE_DISABLED ? VK_CULL_MODE_NONE :
                                         standardMaterial->cullMode == CULLMODE_BACK ? VK_CULL_MODE_BACK_BIT
                                                                                     : VK_CULL_MODE_FRONT_BIT);
                    } else {
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
                    }
                    bindDescriptorSets(commandBuffer, currentFrame);
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount, modelIndex);
                }
                modelIndex += 1;
            }
        }
        vkCmdSetDepthBiasEnable(commandBuffer, VK_FALSE);
    }

    void ShadowMapRenderer::createDescriptorSetLayout() {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                .build();

        // Global UBO
        createUniformBuffers(globalBuffers, sizeof(GlobalUniform));

        // Models UBO
        VkDeviceSize modelBufferSize = sizeof(ModelUniform) * meshes.size();
        createUniformBuffers(modelsBuffers, modelBufferSize);

        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
                .addBinding(0, // global UBO
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(1, // model UBO
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT)
            .build();

        for (uint32_t i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GlobalUniform));
            auto modelBufferInfo = modelsBuffers[i]->descriptorInfo(modelBufferSize);
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeBuffer(1, &modelBufferInfo)
                .build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void ShadowMapRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        vulkanDevice.transitionImageLayout(commandBuffer, shadowMap->getImage(),
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                           0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                           VK_IMAGE_ASPECT_DEPTH_BIT);
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = shadowMap->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = depthClearValue,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0},
                               {shadowMap->size, shadowMap->size}},
                .layerCount = 1,
                .colorAttachmentCount = 0,
                .pColorAttachments = nullptr,
                .pDepthAttachment = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void ShadowMapRenderer::endRendering(VkCommandBuffer commandBuffer, bool isLast) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(
                commandBuffer, shadowMap->getImage(),
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT, // After depth writes
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // Before depth reads in the shader
                VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void ShadowMapRenderer::createImagesResources() {
        shadowMap->createImagesResources();
    }

    void ShadowMapRenderer::cleanupImagesResources() {
        if (shadowMap != nullptr) shadowMap->cleanupImagesResources();
    }

    void ShadowMapRenderer::recreateImagesResources() {

    }

}