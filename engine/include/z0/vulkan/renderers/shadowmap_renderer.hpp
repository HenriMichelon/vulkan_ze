#pragma once

#include "base_renderpass.hpp"
#include "z0/vulkan/framebuffers/shadow_map.hpp"
#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    class ShadowMapRenderer: public BaseRenderpass, public VulkanRenderer {
    public:
        struct GlobalUniform {
            glm::mat4 lightSpace;
        };
        struct ModelUniform {
            glm::mat4 matrix;
        };

        ShadowMapRenderer(VulkanDevice& device, const std::string& shaderDirectory);

        void loadScene(std::shared_ptr<ShadowMap>& shadowMap, std::vector<MeshInstance*>& meshes);
        void cleanup() override;

        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        const float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        const float depthBiasSlope = 1.75f;

        std::vector<MeshInstance*> meshes {};
        std::shared_ptr<ShadowMap> shadowMap;
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBufferw) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

    public:
        ShadowMapRenderer(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer &operator=(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer(const ShadowMapRenderer&&) = delete;
        ShadowMapRenderer &&operator=(const ShadowMapRenderer&&) = delete;
    };

}