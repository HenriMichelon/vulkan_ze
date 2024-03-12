// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
// https://github.com/SaschaWillems/Vulkan/tree/master/examples/shadowmapping
#include "z0/vulkan/shadowmap_renderer.hpp"
#include "z0/log.hpp"

namespace z0 {

    ShadowmapRenderer::ShadowmapRenderer(VulkanDevice &dev,
                                 const std::string& sDir) :
         VulkanRenderer{dev, sDir}
     {
         createImagesResources();
     }

    ShadowmapRenderer::~ShadowmapRenderer() {
        vkDeviceWaitIdle(device);
        cleanupImagesResources();
    }

    void ShadowmapRenderer::loadScene(std::shared_ptr<Node>& rootNode) {
        loadNode(rootNode);
        createResources();
    }

    void ShadowmapRenderer::loadNode(std::shared_ptr<Node>& parent) {
        if (directionalLight == nullptr) {
            if (auto light = dynamic_cast<DirectionalLight*>(parent.get())) {
                directionalLight = light;
                log("Using directional light", directionalLight->toString());
            }
        }
        if (auto omniLight = dynamic_cast<OmniLight *>(parent.get())) {
            omniLights.push_back(omniLight);
        }
        if (auto meshInstance = dynamic_cast<MeshInstance*>(parent.get())) {
            meshes.push_back(meshInstance);
        }
        for(auto& child: parent->getChildren()) {
            loadScene(child);
        }
    }

    void ShadowmapRenderer::loadShaders() {
        vertShader = createShader("shadowmap.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
    }

    void ShadowmapRenderer::update() {
        if (omniLights.empty() || directionalLight == nullptr) return;

        GlobalUniformBufferObject globalUbo {};
        for(int i=0; i < omniLights.size(); i++) {
            /*pointLightsArray[i].position = omniLights[i]->getPosition();
            pointLightsArray[i].color = omniLights[i]->getColorAndIntensity();
            pointLightsArray[i].specular = omniLights[i]->getSpecularIntensity();
            pointLightsArray[i].constant = omniLights[i]->getAttenuation();
            pointLightsArray[i].linear = omniLights[i]->getLinear();
            pointLightsArray[i].quadratic = omniLights[i]->getQuadratic();
            if (auto spot = dynamic_cast<SpotLight*>(omniLights[i])) {
                pointLightsArray[i].isSpot = true;
                pointLightsArray[i].direction = spot->getDirection();
                pointLightsArray[i].cutOff = spot->getCutOff();
                pointLightsArray[i].outerCutOff =spot->getOuterCutOff();
            }*/
            XX
        }
        writeUniformBuffer(globalBuffers, &globalUbo);
    }

    void ShadowmapRenderer::recordCommands(VkCommandBuffer commandBuffer) {
        if (omniLights.empty() || directionalLight == nullptr) return;
        bindShader(commandBuffer, *vertShader);
        for (const auto&meshInstance: meshes) {
            auto mesh = meshInstance->getMesh();
            if (mesh->isValid()) {
                for (const auto& surface: mesh->getSurfaces()) {
                    bindDescriptorSets(commandBuffer);
                    mesh->_getModel()->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                }
            }
        }
    }

    void ShadowmapRenderer::createDescriptorSetLayout() {
        if (omniLights.empty() || directionalLight == nullptr) return;
        globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT) // global UBO
                .build();

        // Global UBO
        createUniformBuffers(globalBuffers, sizeof(GlobalUniformBufferObject));

        globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
            .addBinding(0, // global UBO
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    VK_SHADER_STAGE_ALL_GRAPHICS)
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

    void ShadowmapRenderer::createImagesResources() {
        VkFormat colorFormat = vulkanDevice.getSwapChainImageFormat();
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height,
                                 1, vulkanDevice.getSamples(), colorFormat,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 colorImage, colorImageMemory);
        colorImageView = vulkanDevice.createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

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

        colorImageResolve.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        colorImageResolve.srcOffset = {0, 0, 0};
        colorImageResolve.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        colorImageResolve.dstOffset = {0, 0, 0};
        colorImageResolve.extent = {
                vulkanDevice.getSwapChainExtent().width,
                vulkanDevice.getSwapChainExtent().height,
                1};
    }

    void ShadowmapRenderer::cleanupImagesResources() {
        vkDestroyImageView(device, shadowmapImageView, nullptr);
        vkDestroyImage(device, shadowmapImage, nullptr);
        vkFreeMemory(device, shadowmapImageMemory, nullptr);
    }

    void ShadowmapRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           colorImage,
                                           vulkanDevice.getSwapChainImageFormat(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        // Color attachement : where the rendering is done (multisampled memory image)
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorImageView,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = depthImageView,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
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

    void ShadowmapRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vkCmdEndRendering(commandBuffer);

        vkCmdResolveImage(commandBuffer,
                          colorImage,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          vulkanDevice.getSwapChainImages()[imageIndex],
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          1,
                          &imageResolve);

        vulkanDevice.transitionImageLayout(
                commandBuffer,
                vulkanDevice.getSwapChainImages()[imageIndex],
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }



}