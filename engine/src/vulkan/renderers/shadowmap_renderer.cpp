// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
// https://github.com/SaschaWillems/Vulkan/tree/master/examples/shadowmapping
#include "z0/vulkan/renderers/shadowmap_renderer.hpp"
#include "z0/log.hpp"
#include "z0/nodes/spot_light.hpp"

namespace z0 {

    ShadowMapRenderer::ShadowMapRenderer(VulkanDevice &dev,
                                         const std::string& sDir) :
         VulkanRenderer{dev, sDir, false}
     {
         createResources();
     }

    ShadowMapRenderer::~ShadowMapRenderer() {
        vkDeviceWaitIdle(device);
    }

    void ShadowMapRenderer::loadScene(std::shared_ptr<ShadowMap>& _shadowMap, std::vector<MeshInstance*>& _meshes) {
        meshes = _meshes;
        shadowMap = _shadowMap;
    }

    void ShadowMapRenderer::loadShaders() {
        vertShader = createShader("shadowmap.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
    }

    void ShadowMapRenderer::update() {
        auto depthProjectionMatrix = glm::perspective(shadowMap->getLight()->getOuterCutOff(), 1.0f, zNear, zFar);
        auto depthViewMatrix = glm::lookAt(shadowMap->getLight()->getPosition(), glm::vec3(0.0f), glm::vec3(0, -1, 0));
        auto depthModelMatrix = glm::mat4(1.0f);
        GlobalUniformBufferObject globalUbo { depthProjectionMatrix * depthViewMatrix * depthModelMatrix };
        writeUniformBuffer(globalBuffers, &globalUbo);
    }

    void ShadowMapRenderer::recordCommands(VkCommandBuffer commandBuffer) {
        bindShader(commandBuffer, *vertShader);
        VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_FRAGMENT_BIT};
        vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
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
                    bindDescriptorSets(commandBuffer);
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                }
            }
        }
    }

    void ShadowMapRenderer::createDescriptorSetLayout() {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT) // global UBO
                .build();

        // Global UBO
        createUniformBuffers(globalBuffers, sizeof(GlobalUniformBufferObject));

        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0, // global UBO
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_SHADER_STAGE_VERTEX_BIT)
            .build();

        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GlobalUniformBufferObject));
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void ShadowMapRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           shadowMap->getImage(),
                                           shadowMap->format,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = shadowMap->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
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

    void ShadowMapRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vkCmdEndRendering(commandBuffer);
        /*vulkanDevice.transitionImageLayout(
                commandBuffer,
                shadowmapImage,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);*/
    }

    void ShadowMapRenderer::createImagesResources() {
        shadowMap->createImagesResources();
    }

    void ShadowMapRenderer::cleanupImagesResources() {
        shadowMap->cleanupImagesResources();
    }



}