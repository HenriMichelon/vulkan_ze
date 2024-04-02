#include "z0/vulkan/renderers/depth_prepass_renderer.hpp"
#include "z0/log.hpp"

#include <array>

namespace z0 {

    DepthPrepassRenderer::DepthPrepassRenderer(VulkanDevice &dev, const std::string& sDir) : BaseMeshesRenderer{dev, sDir}{
    }

    void DepthPrepassRenderer::loadScene(std::shared_ptr<DepthBuffer>& _depthBuffer,
                                         Camera* _camera,
                                         std::vector<MeshInstance*>& _meshes) {
        meshes = _meshes;
        depthBuffer = _depthBuffer;
        currentCamera = _camera;
        createResources();
    }

    void DepthPrepassRenderer::loadShaders() {
        vertShader = createShader("depth_prepass.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
    }

    void DepthPrepassRenderer::update(uint32_t currentFrame) {
        if (meshes.empty() || currentCamera == nullptr) return;
        GlobalUniformBufferObject globalUbo {
            .projection = currentCamera->getProjection(),
            .view = currentCamera->getView()
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);

        uint32_t modelIndex = 0;
        for (const auto&meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                ModelUniformBufferObject modelUbo{
                    .matrix = meshInstance->getTransformGlobal(),
                };
                writeUniformBuffer(modelsBuffers, currentFrame, &modelUbo, modelIndex);
            }
            modelIndex += 1;
        }
    }

    void DepthPrepassRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (meshes.empty() || currentCamera == nullptr) return;
        setInitialState(commandBuffer);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);

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
                    std::array<uint32_t, 2> offsets = {
                        0, // globalBuffers
                        static_cast<uint32_t>(modelsBuffers[currentFrame]->getAlignmentSize() * modelIndex),
                    };
                    bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                }
            }
            modelIndex += 1;
        }
    }

    void DepthPrepassRenderer::createDescriptorSetLayout() {
        if (meshes.empty() || currentCamera == nullptr) return;
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                .build();

        // Global UBO
        createUniformBuffers(globalBuffers, sizeof(GlobalUniformBufferObject));

        // Models UBO
        VkDeviceSize modelBufferSize = sizeof(ModelUniformBufferObject);
        createUniformBuffers(modelsBuffers, modelBufferSize, meshes.size());;

        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0, // global UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(1, // model UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_VERTEX_BIT)
            .build();

        for (uint32_t i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GlobalUniformBufferObject));
            auto modelBufferInfo = modelsBuffers[i]->descriptorInfo(modelBufferSize);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeBuffer(1, &modelBufferInfo)
                .build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void DepthPrepassRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        vulkanDevice.transitionImageLayout(
                commandBuffer,
               depthBuffer->getImage(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               0, // no need to wait for any prior operation (VK_IMAGE_LAYOUT_UNDEFINED))
               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // as soon as possible
               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               VK_IMAGE_ASPECT_DEPTH_BIT);

        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = depthBuffer->getImageView(),
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
                               {vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height}},
                .layerCount = 1,
                .colorAttachmentCount = 0,
                .pColorAttachments = nullptr,
                .pDepthAttachment = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void DepthPrepassRenderer::endRendering(VkCommandBuffer commandBuffer, bool isLast) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(
                commandBuffer,depthBuffer->getImage(),
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void DepthPrepassRenderer::createImagesResources() {
        depthBuffer->createImagesResources();
    }

    void DepthPrepassRenderer::cleanupImagesResources() {
        if (depthBuffer != nullptr) depthBuffer->cleanupImagesResources();
    }

    void DepthPrepassRenderer::recreateImagesResources() {
        depthBuffer->cleanupImagesResources();
        depthBuffer->createImagesResources();
    }

}