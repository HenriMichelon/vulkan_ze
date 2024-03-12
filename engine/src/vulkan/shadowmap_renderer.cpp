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
            //XX
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
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // For shadow mapping we only need a depth attachment
        VkFormat colorFormat = vulkanDevice.getSwapChainImageFormat();
        vulkanDevice.createImage(shadowMapize, shadowMapize,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 shadowmapDepthFormat,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 shadowmapImage, shadowmapImageMemory);
        shadowmapImageView = vulkanDevice.createImageView(shadowmapImage,
                                                          shadowmapDepthFormat,
                                                          VK_IMAGE_ASPECT_DEPTH_BIT,
                                                          1);

        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        VkFilter shadowmap_filter = vulkanDevice.formatIsFilterable( shadowmapDepthFormat, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        VkSamplerCreateInfo samplerCreateInfo {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.magFilter = shadowmap_filter;
        samplerCreateInfo.minFilter = shadowmap_filter;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
        samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerCreateInfo, nullptr, &shadowmapSampler) != VK_SUCCESS) {
            die("failed to create shadowmap sampler!");
        }
    }

    void ShadowmapRenderer::cleanupImagesResources() {
        vkDestroySampler(device, shadowmapSampler, nullptr);
        vkDestroyImageView(device, shadowmapImageView, nullptr);
        vkDestroyImage(device, shadowmapImage, nullptr);
        vkFreeMemory(device, shadowmapImageMemory, nullptr);
    }

    void ShadowmapRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vulkanDevice.transitionImageLayout(commandBuffer,
                                           shadowmapImage,
                                           shadowmapDepthFormat,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = shadowmapImageView,
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
                               {shadowMapize, shadowMapize}},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void ShadowmapRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vkCmdEndRendering(commandBuffer);
        /*vulkanDevice.transitionImageLayout(
                commandBuffer,
                shadowmapImage,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);*/
    }



}