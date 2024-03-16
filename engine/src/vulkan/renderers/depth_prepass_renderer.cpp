#include "z0/vulkan/renderers/depth_prepass_renderer.hpp"
#include "z0/log.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {

    DepthPrepassRenderer::DepthPrepassRenderer(VulkanDevice &dev,
                                         const std::string& sDir) :
            BaseRenderer{dev, sDir}
     {
     }

    void DepthPrepassRenderer::cleanup() {
        depthBuffer.reset();
        modelsBuffers.clear();
        BaseRenderer::cleanup();
    }
    void DepthPrepassRenderer::loadScene(std::shared_ptr<DepthBuffer>& _depthBuffer,
                                         Camera* _camera,
                                         std::vector<MeshInstance*>& _meshes) {
        meshes = _meshes;
        depthBuffer = _depthBuffer;
        camera = _camera;
        createResources();
    }

    void DepthPrepassRenderer::loadShaders() {
        vertShader = createShader("depth_prepass.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
    }

    void DepthPrepassRenderer::update(uint32_t currentFrame) {
        if (meshes.empty() || camera == nullptr) return;
        GlobalUniformBufferObject globalUbo {
            .projection = camera->getProjection(),
            .view = camera->getView()
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);

        uint32_t modelIndex = 0;
        for (const auto&meshInstance: meshes) {
            ModelUniformBufferObject modelUbo{
                .matrix = meshInstance->getGlobalTransform(),
            };
            writeUniformBuffer(modelsBuffers, currentFrame, &modelUbo, modelIndex);
            modelIndex += 1;
        }
    }

    void DepthPrepassRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (meshes.empty() || camera == nullptr) return;
        bindShader(commandBuffer, *vertShader);
        VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_FRAGMENT_BIT};
        vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);

        vkCmdSetRasterizationSamplesEXT(commandBuffer, vulkanDevice.getSamples());
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);

        setViewport(commandBuffer, vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height);

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

        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GlobalUniformBufferObject));
            auto modelBufferInfo = modelsBuffers[i]->descriptorInfo(modelBufferSize);
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeBuffer(1, &modelBufferInfo)
                .build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void DepthPrepassRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           depthBuffer->getImage(),
                                           depthBuffer->getFormat(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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

    void DepthPrepassRenderer::endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) {
        vkCmdEndRendering(commandBuffer);
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = depthBuffer->getImage();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
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