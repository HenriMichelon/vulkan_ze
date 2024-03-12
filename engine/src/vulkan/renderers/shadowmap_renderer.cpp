// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
// https://github.com/SaschaWillems/Vulkan/tree/master/examples/shadowmapping
#include "z0/vulkan/renderers/shadowmap_renderer.hpp"
#include "z0/log.hpp"

namespace z0 {

    ShadowmapRenderer::ShadowmapRenderer(VulkanDevice &dev,
                                 const std::string& sDir) :
         VulkanRenderer{dev, sDir}
     {
         createImagesResources();
         createResources();
     }

    ShadowmapRenderer::~ShadowmapRenderer() {
        vkDeviceWaitIdle(device);
        cleanupImagesResources();
    }

    std::shared_ptr<ShadowMap> ShadowmapRenderer::loadScene(const std::shared_ptr<Light>& _light, std::vector<MeshInstance*>& _meshes) {
        light = _light;
        _meshes = meshes;
        shadowMap = std::make_shared<ShadowMap>(vulkanDevice);
        createImagesResources();
        return shadowMap;
    }

    void ShadowmapRenderer::loadShaders() {
        vertShader = createShader("shadowmap.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
    }

    void ShadowmapRenderer::update() {
        GlobalUniformBufferObject globalUbo {};
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
        writeUniformBuffer(globalBuffers, &globalUbo);
    }

    void ShadowmapRenderer::recordCommands(VkCommandBuffer commandBuffer) {
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

    void ShadowmapRenderer::beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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

    void ShadowmapRenderer::endRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        vkCmdEndRendering(commandBuffer);
        /*vulkanDevice.transitionImageLayout(
                commandBuffer,
                shadowmapImage,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);*/
    }

    void ShadowmapRenderer::createImagesResources() {
        shadowMap->createImagesResources();
    }

    void ShadowmapRenderer::cleanupImagesResources() {
        shadowMap->cleanupImagesResources();
    }



}