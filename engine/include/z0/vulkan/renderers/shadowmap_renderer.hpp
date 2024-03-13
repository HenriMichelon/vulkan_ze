#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/renderers/shadowmap.hpp"
#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    class ShadowMapRenderer: public VulkanRenderer {
    public:
        struct GlobalUniformBufferObject {
            glm::mat4 lightSpace;
        };
        struct ModelUniformBufferObject {
            glm::mat4 matrix;
        };

        ShadowMapRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~ShadowMapRenderer();

        void loadScene(std::shared_ptr<ShadowMap>& shadowMap, std::vector<MeshInstance*>& meshes);

    private:
        // Keep depth range as small as possible
        // for better shadow map precision const
        const float zNear = .1f;
        const float zFar = 100.0f;
        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        const float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        const float depthBiasSlope = 1.75f;

        std::vector<MeshInstance*> meshes {};
        std::shared_ptr<ShadowMap> shadowMap;
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};

        void update() override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex) override;

    public:
        ShadowMapRenderer(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer &operator=(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer(const ShadowMapRenderer&&) = delete;
        ShadowMapRenderer &&operator=(const ShadowMapRenderer&&) = delete;
    };

}