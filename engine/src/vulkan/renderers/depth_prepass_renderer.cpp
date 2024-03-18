#include "z0/vulkan/renderers/depth_prepass_renderer.hpp"
#include "z0/log.hpp"


namespace z0 {

    DepthPrepassRenderer::DepthPrepassRenderer(VulkanDevice &dev, const std::string& sDir) : BaseMeshesRenderer{dev, sDir}{
    }

    void DepthPrepassRenderer::loadScene(std::shared_ptr<DepthBuffer>& _depthBuffer,
                                         Camera* _camera,
                                         std::vector<MeshInstance*>& _meshes,
                                         std::map<Resource::rid_t, int32_t>& _imagesIndices,
                                         std::unordered_set<std::shared_ptr<VulkanImage>>& _images) {
        meshes = _meshes;
        depthBuffer = _depthBuffer;
        currentCamera = _camera;
        imagesIndices = _imagesIndices;
        images = _images;
        createResources();
    }

    void DepthPrepassRenderer::loadShaders() {
        vertShader = createShader("depth_prepass.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("depth_prepass.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void DepthPrepassRenderer::update(uint32_t currentFrame) {
        if (meshes.empty() || currentCamera == nullptr) return;
        GlobalUniformBufferObject globalUbo {
            .projection = currentCamera->getProjection(),
            .view = currentCamera->getView()
        };
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);

        uint32_t modelIndex = 0;
        int32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                ModelUniformBufferObject modelUbo{
                    .matrix = meshInstance->getGlobalTransform(),
                };
                writeUniformBuffer(modelsBuffers, currentFrame, &modelUbo, modelIndex);
                for (const auto &surface: meshInstance->getMesh()->getSurfaces()) {
                    SurfaceUniformBufferObject surfaceUbo { };
                    if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                        surfaceUbo.albedoColor = standardMaterial->albedoColor.color;
                        if (standardMaterial->albedoTexture != nullptr) {
                            surfaceUbo.diffuseIndex = imagesIndices[standardMaterial->albedoTexture->getImage().getId()];
                        }
                        surfaceUbo.transparency = standardMaterial->transparency;
                    }
                    writeUniformBuffer(surfacesBuffers, currentFrame, &surfaceUbo, surfaceIndex);
                    surfaceIndex += 1;
                }
            }
            modelIndex += 1;
        }
    }

    void DepthPrepassRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (meshes.empty() || currentCamera == nullptr) return;
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);

        uint32_t modelIndex = 0;
        int32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            auto mesh = meshInstance->getMesh();
            if (mesh->isValid()) {
                for (const auto& surface: mesh->getSurfaces()) {
                    if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                        if (standardMaterial->transparency != TRANSPARENCY_DISABLED) {
                            surfaceIndex += 1;
                            break;
                        }
                        vkCmdSetCullMode(commandBuffer,
                                         standardMaterial->cullMode == CULLMODE_DISABLED ? VK_CULL_MODE_NONE :
                                         standardMaterial->cullMode == CULLMODE_BACK ? VK_CULL_MODE_BACK_BIT
                                                                                     : VK_CULL_MODE_FRONT_BIT);
                    } else {
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
                    }
                    std::array<uint32_t, 3> offsets = {
                        0, // globalBuffers
                        static_cast<uint32_t>(modelsBuffers[currentFrame]->getAlignmentSize() * modelIndex),
                        static_cast<uint32_t>(surfacesBuffers[currentFrame]->getAlignmentSize() * surfaceIndex),
                    };
                    bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    surfaceIndex += 1;
                }
            }
            modelIndex += 1;
        }
    }

    void DepthPrepassRenderer::createDescriptorSetLayout() {
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT) // textures
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // surface UBO
                .build();

        // Global UBO
        createUniformBuffers(globalBuffers, sizeof(GlobalUniformBufferObject));

        // Models UBO
        VkDeviceSize modelBufferSize = sizeof(ModelUniformBufferObject);
        createUniformBuffers(modelsBuffers, modelBufferSize, meshes.size());;

        // Surface UBO
        VkDeviceSize surfaceBufferSize = sizeof(SurfaceUniformBufferObject);
        createUniformBuffers(surfacesBuffers, surfaceBufferSize, meshes.size());;

        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0, // global UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(1, // textures
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        images.size())
            .addBinding(2, // model UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(3, // surface UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GlobalUniformBufferObject));
            auto modelBufferInfo = modelsBuffers[i]->descriptorInfo(modelBufferSize);
            auto surfaceBufferInfo = surfacesBuffers[i]->descriptorInfo(surfaceBufferSize);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            for(const auto& image : images) {
                imagesInfo.push_back(image->imageInfo());
            }
            if (!VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeImage(1, imagesInfo.data())
                .writeBuffer(2, &modelBufferInfo)
                .writeBuffer(3, &surfaceBufferInfo)
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

    void DepthPrepassRenderer::endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) {
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