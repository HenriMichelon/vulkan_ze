#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/vulkan/renderers/shadowmap.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/nodes/light.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/omni_light.hpp"

namespace z0 {

    class ShadowmapRenderer: public VulkanRenderer {
    public:
        struct GlobalUniformBufferObject {
            glm::mat4 depthMVP;
        };

        ShadowmapRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~ShadowmapRenderer();

        std::shared_ptr<ShadowMap> loadScene(const std::shared_ptr<Light>& light, std::vector<MeshInstance*>& meshes);

    private:
        std::shared_ptr<Light> light;
        std::vector<MeshInstance*> meshes {};
        std::shared_ptr<ShadowMap> shadowMap;

        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;
        std::unique_ptr<VulkanDescriptorPool> globalPool {};
        std::vector<std::unique_ptr<VulkanBuffer>> globalBuffers{MAX_FRAMES_IN_FLIGHT};

        void update() override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex) override;

    public:
        ShadowmapRenderer(const ShadowmapRenderer&) = delete;
        ShadowmapRenderer &operator=(const ShadowmapRenderer&) = delete;
        ShadowmapRenderer(const ShadowmapRenderer&&) = delete;
        ShadowmapRenderer &&operator=(const ShadowmapRenderer&&) = delete;
    };

}