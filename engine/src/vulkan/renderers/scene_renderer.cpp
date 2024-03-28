/*
 *  Using one descriptor per scene with offsets
 * https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
 */
#include "z0/vulkan/renderers/scene_renderer.hpp"
#include "z0/nodes/skybox.hpp"
#include "z0/log.hpp"

#include <glm/gtc/matrix_inverse.hpp>

#include <array>
#include <set>

namespace z0 {

    SceneRenderer::SceneRenderer(VulkanDevice &dev, std::string sDir) : BaseMeshesRenderer{dev, sDir} {
         createImagesResources();
     }

    void SceneRenderer::cleanup() {
        for (const auto& shadowMapRenderer : shadowMapRenderers) {
            shadowMapRenderer->cleanup();
        }
        if (skyboxRenderer != nullptr) skyboxRenderer->cleanup();
        shadowMapRenderers.clear();
        shadowMaps.clear();
        opaquesMeshes.clear();
        transparentsMeshes.clear();
        depthPrepassRenderer->cleanup();
        images.clear();
        shadowMapsBuffers.clear();
        surfacesBuffers.clear();
        pointLightBuffers.clear();
        BaseMeshesRenderer::cleanup();
    }

    void SceneRenderer::loadScene(std::shared_ptr<Node>& rootNode) {
        loadNode(rootNode);
        createImagesIndex(rootNode);

        // Build indices collections for uniform buffer to model & surfaces relations
        // Build transparent objets sorted collection
        std::multiset<DistanceSortedNode> sortedTransparentNodes;
        uint32_t surfaceIndex = 0;
        uint32_t modelIndex = 0;
        for (const auto &meshInstance: meshes) {
            modelIndices[meshInstance->getId()] = modelIndex;
            auto transparent = false;
            for (const auto &material: meshInstance->getMesh()->_getMaterials()) {
                surfacesIndices[material->getId()] = surfaceIndex;
                surfaceIndex += 1;
                if (auto* standardMaterial = dynamic_cast<StandardMaterial*>(material.get())) {
                    if (standardMaterial->transparency != TRANSPARENCY_DISABLED) {
                        transparent = true;
                    }
                }
            }
            if (transparent) {
                sortedTransparentNodes.insert(DistanceSortedNode(*meshInstance, *currentCamera));
            } else {
                opaquesMeshes.push_back(meshInstance);
            }
            modelIndex += 1;
        }
        for (const auto& node: sortedTransparentNodes) {
            transparentsMeshes.push_back(dynamic_cast<MeshInstance*>(&node.getNode()));
        }

        createResources();

        for (auto& shadowMap : shadowMaps) {
            auto shadowMapRenderer = std::make_shared<ShadowMapRenderer>(vulkanDevice, shaderDirectory);
            shadowMapRenderer->loadScene(shadowMap, meshes);
            shadowMapRenderers.push_back(shadowMapRenderer);
            vulkanDevice.registerRenderer(shadowMapRenderer);
        }
        depthPrepassRenderer->loadScene(depthBuffer, currentCamera, opaquesMeshes);
        vulkanDevice.registerRenderer(depthPrepassRenderer);
    }

    void SceneRenderer::loadNode(std::shared_ptr<Node>& parent) {
        if (currentCamera == nullptr) {
            if (auto* camera = dynamic_cast<Camera*>(parent.get())) {
                currentCamera = camera;
                log("Using camera", currentCamera->toString());
            }
        }
        if (skyboxRenderer == nullptr) {
            if (auto* skybox = dynamic_cast<Skybox*>(parent.get())) {
                skyboxRenderer = std::make_unique<SkyboxRenderer>(vulkanDevice, shaderDirectory);
                skyboxRenderer->loadScene(skybox->getCubemap()->_getCubemap());
                log("Using skybox", skybox->toString());
            }
        }
        if (directionalLight == nullptr) {
            if (auto* light = dynamic_cast<DirectionalLight*>(parent.get())) {
                directionalLight = light;
                log("Using directional light", directionalLight->toString());
                if (directionalLight->getCastShadows()) {
                    shadowMaps.push_back(std::make_shared<ShadowMap>(vulkanDevice, directionalLight));
                }
            }
        }
        if (environement == nullptr) {
            if (auto* env = dynamic_cast<Environment*>(parent.get())) {
                environement = env;
                log("Using environment", environement->toString());
            }
        }
        if (auto* omniLight = dynamic_cast<OmniLight*>(parent.get())) {
            omniLights.push_back(omniLight);
            if (omniLight->getCastShadows()) {
                if (auto *spotLight = dynamic_cast<SpotLight *>(parent.get())) {
                    shadowMaps.push_back(std::make_shared<ShadowMap>(vulkanDevice, spotLight));
                }
            }
        }
        createImagesList(parent);
        for(auto& child: parent->getChildren()) {
            loadNode(child);
        }
    }

    void SceneRenderer::createImagesList(std::shared_ptr<Node>& node) {
        if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            meshes.push_back(meshInstance);
            for(const auto& material : meshInstance->getMesh()->_getMaterials()) {
                if (auto standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                    if (standardMaterial->albedoTexture != nullptr) {
                        images.insert(standardMaterial->albedoTexture->getImage()._getImage());
                    }
                    if (standardMaterial->specularTexture != nullptr) {
                        images.insert(standardMaterial->specularTexture->getImage()._getImage());
                    }
                    if (standardMaterial->normalTexture != nullptr) {
                        images.insert(standardMaterial->normalTexture->getImage()._getImage());
                    }
                }
            }
        }
    }

    void SceneRenderer::createImagesIndex(std::shared_ptr<Node>& node) {
        if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            for(const auto& material : meshInstance->getMesh()->_getMaterials()) {
                if (auto* standardMaterial = dynamic_cast<StandardMaterial*>(material.get())) {
                    if (standardMaterial->albedoTexture != nullptr) {
                        auto &image = standardMaterial->albedoTexture->getImage();
                        auto index = std::distance(std::begin(images), images.find(image._getImage()));
                        imagesIndices[image.getId()] = static_cast<int32_t>(index);
                    }
                    if (standardMaterial->specularTexture != nullptr) {
                        auto &image = standardMaterial->specularTexture->getImage();
                        auto index = std::distance(std::begin(images), images.find(image._getImage()));
                        imagesIndices[image.getId()] = static_cast<int32_t>(index);
                    }
                    if (standardMaterial->normalTexture != nullptr) {
                        auto &image = standardMaterial->normalTexture->getImage();
                        auto index = std::distance(std::begin(images), images.find(image._getImage()));
                        imagesIndices[image.getId()] = static_cast<int32_t>(index);
                    }
                }
            }
        }
        for(auto& child: node->getChildren()) {
            createImagesIndex(child);
        }
    }

    void SceneRenderer::loadShaders() {
        if (skyboxRenderer != nullptr) skyboxRenderer->loadShaders();
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SceneRenderer::update(uint32_t currentFrame) {
        if (currentCamera == nullptr) return;
        if (skyboxRenderer != nullptr) skyboxRenderer->update(currentCamera, currentFrame);
        if (meshes.empty() ) return;

        GobalUniformBufferObject globalUbo{
            .projection = currentCamera->getProjection(),
            .view = currentCamera->getView(),
            .cameraPosition = currentCamera->getPosition(),
            .shadowMapsCount = static_cast<uint32_t>(shadowMaps.size()),
        };

        // TODO if empty
        auto shadowMapArray =  std::make_unique<ShadowMapUniform[]>(globalUbo.shadowMapsCount);
        for(int i=0; i < globalUbo.shadowMapsCount; i++) {
            shadowMapArray[i].lightSpace = shadowMaps[i]->getLightSpace();
            shadowMapArray[i].lightPos = shadowMaps[i]->getLightPosition();
        }
        writeUniformBuffer(shadowMapsBuffers, currentFrame, shadowMapArray.get());

        if (directionalLight != nullptr) {
            globalUbo.directionalLight = {
                .direction = directionalLight->getDirection(),
                .color = directionalLight->getColorAndIntensity(),
                .specular = directionalLight->getSpecularIntensity(),
            };
            globalUbo.haveDirectionalLight = true;
        }
        if (environement != nullptr) {
            globalUbo.ambient = environement->getAmbientColorAndIntensity();
        }
        globalUbo.pointLightsCount = omniLights.size();
        writeUniformBuffer(globalBuffers, currentFrame, &globalUbo);

        // TODO if empty
        auto pointLightsArray =  std::make_unique<PointLightUniform[]>(globalUbo.pointLightsCount);
        for(int i=0; i < globalUbo.pointLightsCount; i++) {
            pointLightsArray[i].position = omniLights[i]->getPosition();
            pointLightsArray[i].color = omniLights[i]->getColorAndIntensity();
            pointLightsArray[i].specular = omniLights[i]->getSpecularIntensity();
            pointLightsArray[i].constant = omniLights[i]->getAttenuation();
            pointLightsArray[i].linear = omniLights[i]->getLinear();
            pointLightsArray[i].quadratic = omniLights[i]->getQuadratic();
            if (auto* spot = dynamic_cast<SpotLight*>(omniLights[i])) {
                pointLightsArray[i].isSpot = true;
                pointLightsArray[i].direction = spot->getDirection();
                pointLightsArray[i].cutOff = spot->getCutOff();
                pointLightsArray[i].outerCutOff =spot->getOuterCutOff();
            }
        }
        writeUniformBuffer(pointLightBuffers, currentFrame, pointLightsArray.get());

        uint32_t modelIndex = 0;
        uint32_t surfaceIndex = 0;
        for (const auto&meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                ModelUniformBufferObject modelUbo {
                    .matrix = meshInstance->getTransformGlobal(),
                };
                writeUniformBuffer(modelsBuffers, currentFrame, &modelUbo, modelIndex);
                for (const auto &surface: meshInstance->getMesh()->getSurfaces()) {
                    SurfaceUniformBufferObject surfaceUbo { };
                    if (auto* standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                        surfaceUbo.albedoColor = standardMaterial->albedoColor.color;
                        if (standardMaterial->albedoTexture != nullptr) {
                            surfaceUbo.diffuseIndex = imagesIndices[standardMaterial->albedoTexture->getImage().getId()];
                        }
                        if (standardMaterial->specularTexture != nullptr) {
                            surfaceUbo.specularIndex = imagesIndices[standardMaterial->specularTexture->getImage().getId()];
                        }
                        if (standardMaterial->normalTexture != nullptr) {
                            surfaceUbo.normalIndex = imagesIndices[standardMaterial->normalTexture->getImage().getId()];
                        }
                        surfaceUbo.transparency = standardMaterial->transparency;
                        surfaceUbo.alphaScissor = standardMaterial->alphaScissor;
                    }
                    writeUniformBuffer(surfacesBuffers, currentFrame, &surfaceUbo, surfaceIndex);
                    surfaceIndex += 1;
                }
            }
            modelIndex += 1;
        }
    }

    void SceneRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (currentCamera == nullptr) return;
        if (!meshes.empty()) {
            setInitialState(commandBuffer);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE); // we have a depth prepass
            vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_EQUAL); // comparing with the depth prepass
            drawMeshes(commandBuffer, currentFrame, opaquesMeshes);

            vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
            drawMeshes(commandBuffer, currentFrame, transparentsMeshes);
        }
        if (skyboxRenderer != nullptr) skyboxRenderer->recordCommands(commandBuffer, currentFrame);
    }

    void SceneRenderer::drawMeshes(VkCommandBuffer commandBuffer, uint32_t currentFrame, const std::vector<MeshInstance*>& meshesToDraw) {
        for (const auto& meshInstance : meshesToDraw) {
            auto modelIndex = modelIndices[meshInstance->getId()];
            auto mesh = meshInstance->getMesh();
            if (mesh->isValid()) {
                for (const auto& surface: mesh->getSurfaces()) {
                    const auto& material = surface->material.get();
                    if (auto standardMaterial = dynamic_cast<StandardMaterial*>(material)) {
                        vkCmdSetCullMode(commandBuffer,
                                         standardMaterial->cullMode == CULLMODE_DISABLED ? VK_CULL_MODE_NONE :
                                         standardMaterial->cullMode == CULLMODE_BACK ? VK_CULL_MODE_BACK_BIT
                                                                                     : VK_CULL_MODE_FRONT_BIT);
                    } else {
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
                    }
                    auto surfaceIndex = surfacesIndices[material->getId()];
                    std::array<uint32_t, 5> offsets = {
                            0, // globalBuffers
                            static_cast<uint32_t>(modelsBuffers[currentFrame]->getAlignmentSize() * modelIndex),
                            static_cast<uint32_t>(surfacesBuffers[currentFrame]->getAlignmentSize() * surfaceIndex),
                            0, // pointLightBuffers
                            0, // shadowMapsBuffers
                    };
                    bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                }
            }
        }
    }

    void SceneRenderer::createDescriptorSetLayout() {
        if (currentCamera == nullptr) return;
        if (skyboxRenderer != nullptr) skyboxRenderer->createDescriptorSetLayout();
        if (meshes.empty()) return;
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT) // textures
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // surfaces UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // pointlightarray UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT) // shadow map
                .build();

        // Global UBO
        createUniformBuffers(globalBuffers, sizeof(GobalUniformBufferObject));

        // Models UBO
        VkDeviceSize modelBufferSize = sizeof(ModelUniformBufferObject);
        createUniformBuffers(modelsBuffers, modelBufferSize, meshes.size());

        // Surface materials UBO
        VkDeviceSize surfaceBufferSize = sizeof(SurfaceUniformBufferObject);
        uint32_t surfaceCount = 0;
        for (const auto& meshInstance: meshes) {
            if (meshInstance->getMesh()->isValid()) {
                surfaceCount += meshInstance->getMesh()->getSurfaces().size();
            }
        }
        createUniformBuffers(surfacesBuffers, surfaceBufferSize, surfaceCount);

        // PointLight array UBO
        VkDeviceSize pointLightBufferSize = sizeof(PointLightUniform) * (omniLights.size()+ (omniLights.empty() ? 1 : 0));
        createUniformBuffers(pointLightBuffers, pointLightBufferSize);

        // Shadow maps UBO
        VkDeviceSize shadowMapBufferSize = sizeof(ShadowMapUniform) * (shadowMaps.size()+ (shadowMaps.empty() ? 1 : 0));
        createUniformBuffers(shadowMapsBuffers, shadowMapBufferSize);

        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0, // global UBO
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, // textures
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       VK_SHADER_STAGE_FRAGMENT_BIT,
                       images.size())
            .addBinding(2, // model UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(3, // surfaces UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(4, // PointLight array UBO
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(5, // shadow maps infos
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                        VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(6, // shadow maps
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        shadowMaps.size())
           .build();

        for (int i = 0; i < descriptorSets.size(); i++) {
            auto globalBufferInfo = globalBuffers[i]->descriptorInfo(sizeof(GobalUniformBufferObject));
            auto modelBufferInfo = modelsBuffers[i]->descriptorInfo(modelBufferSize);
            auto surfaceBufferInfo = surfacesBuffers[i]->descriptorInfo(surfaceBufferSize);
            auto pointLightBufferInfo = pointLightBuffers[i]->descriptorInfo(pointLightBufferSize);
            auto shadowMapBufferInfo = shadowMapsBuffers[i]->descriptorInfo(shadowMapBufferSize);
            std::vector<VkDescriptorImageInfo> imagesInfo{};
            for(const auto& image : images) {
                imagesInfo.push_back(image->imageInfo());
            }
            auto writer = VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeImage(1, imagesInfo.data())
                .writeBuffer(2, &modelBufferInfo)
                .writeBuffer(3, &surfaceBufferInfo)
                .writeBuffer(4, &pointLightBufferInfo)
                .writeBuffer(5, &shadowMapBufferInfo);
            std::vector<VkDescriptorImageInfo> shadowMapsInfo{};
            if (shadowMaps.empty()) {
                VkDescriptorImageInfo imageInfo = imagesInfo[0]; // find a better solution (blank image ?)
                writer.writeImage(6, &imageInfo);
            } else {
                for (const auto &shadowMap: shadowMaps) {
                    shadowMapsInfo.push_back(VkDescriptorImageInfo{
                        .sampler = shadowMap->getSampler(),
                        .imageView = shadowMap->getImageView(),
                        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                    });
                }
                writer.writeImage(6, shadowMapsInfo.data());
            }
            if (!writer.build(descriptorSets[i])) {
                die("Cannot allocate descriptor set");
            }
        }
    }

    void SceneRenderer::recreateImagesResources() {
        cleanupImagesResources();
        createImagesResources();
    }

    // Create Color Resources (where we draw)
    // https://vulkan-tutorial.com/Multisampling#page_Setting-up-a-render-target
    void SceneRenderer::createImagesResources() {
        // Multisampled offscreen buffer
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height,
                                 1,
                                 vulkanDevice.getSamples(),
                                 renderFormat,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 colorImage, colorImageMemory);
        colorImageView = vulkanDevice.createImageView(colorImage,
                                                      renderFormat,
                                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                                      1);

        // Non multisampled offscreen buffer
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 renderFormat,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 resolvedColorImage, resolvedColorImageMemory);
        resolvedColorImageView = vulkanDevice.createImageView(resolvedColorImage,
                                                      renderFormat,
                                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                                      1);


        // Depth buffer
        if (depthBuffer == nullptr) {
            depthBuffer = std::make_shared<DepthBuffer>(vulkanDevice);
            depthPrepassRenderer = std::make_shared<DepthPrepassRenderer>(vulkanDevice, shaderDirectory);
        } else {
            depthBuffer->createImagesResources();
        }

        // For bliting image to swapchain
        colorImageBlit.srcOffsets[0] = {0, 0, 0 };
        colorImageBlit.srcOffsets[1] = {
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().width),
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().height), 1 };
        colorImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.srcSubresource.mipLevel = 0;
        colorImageBlit.srcSubresource.baseArrayLayer = 0;
        colorImageBlit.srcSubresource.layerCount = 1;
        colorImageBlit.dstOffsets[0] = {0, 0, 0 };
        colorImageBlit.dstOffsets[1] = {
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().width),
                static_cast<int32_t>(vulkanDevice.getSwapChainExtent().height), 1 };
        colorImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.dstSubresource.mipLevel = 0;
        colorImageBlit.dstSubresource.baseArrayLayer = 0;
        colorImageBlit.dstSubresource.layerCount = 1;

    }

    void SceneRenderer::cleanupImagesResources() {
        if (depthBuffer != nullptr) depthBuffer->cleanupImagesResources();
        vkDestroyImageView(device, resolvedColorImageView, nullptr);
        vkDestroyImage(device, resolvedColorImage, nullptr);
        vkFreeMemory(device, resolvedColorImageMemory, nullptr);
        vkDestroyImageView(device, colorImageView, nullptr);
        vkDestroyImage(device, colorImage, nullptr);
        vkFreeMemory(device, colorImageMemory, nullptr);
    }

    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void SceneRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        vulkanDevice.transitionImageLayout(commandBuffer, colorImage,
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        vulkanDevice.transitionImageLayout(commandBuffer, resolvedColorImage,
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        // Color attachement : where the rendering is done (multisampled memory image)
        // Resolved into a non multisampled image
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorImageView,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT ,
                .resolveImageView = resolvedColorImageView,
                .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = depthBuffer->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = depthClearValue,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, vulkanDevice.getSwapChainExtent()},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void SceneRenderer::endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) {
        vkCmdEndRendering(commandBuffer);
        vulkanDevice.transitionImageLayout(
                commandBuffer,swapChainImage,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
        // Blit image to swap chain to change format VK_FORMAT_R16G16B16A16_SFLOAT -> VK_FORMAT_B8G8R8A8_SRGB
        // https://www.reddit.com/r/vulkan/comments/e0hsth/whats_the_best_way_to_copy_between_images_with/
        vkCmdBlitImage(commandBuffer,
                       resolvedColorImage,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       swapChainImage,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1,
                       &colorImageBlit,
                       VK_FILTER_LINEAR );
        vulkanDevice.transitionImageLayout(
                commandBuffer,swapChainImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
    }



}