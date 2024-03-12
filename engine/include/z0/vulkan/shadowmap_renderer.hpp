#pragma once

#include "z0/vulkan/vulkan_renderer.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/nodes/camera.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/nodes/environment.hpp"
#include "z0/nodes/omni_light.hpp"
#include "z0/nodes/spot_light.hpp"

namespace z0 {

    class ShadowmapRenderer: public VulkanRenderer {
    public:
        struct GlobalUniformBufferObject {
            glm::mat4 depthMVP[1]; // One per light
        };

        ShadowmapRenderer(VulkanDevice& device, const std::string& shaderDirectory);
        ~ShadowmapRenderer();

        void loadScene(std::shared_ptr<Node>& rootNode);

    private:
        const VkFormat shadowmapDepthFormat{ VK_FORMAT_D16_UNORM };
        // Shadow map dimension
#if defined(__ANDROID__)
        // Use a smaller size on Android for performance reasons
	    const uint32_t shadowMapize{ 1024 };
#else
        const uint32_t shadowMapize{ 2048 };
#endif

        std::vector<MeshInstance*> meshes {};
        DirectionalLight* directionalLight{nullptr};
        std::vector<OmniLight*> omniLights;

        std::unique_ptr<VulkanShader> vertShader;
        std::unique_ptr<VulkanShader> fragShader;

        std::unique_ptr<VulkanDescriptorPool> globalPool {};
        std::vector<std::unique_ptr<VulkanBuffer>> globalBuffers{MAX_FRAMES_IN_FLIGHT};

        VkImage shadowmapImage;
        VkDeviceMemory shadowmapImageMemory;
        VkImageView shadowmapImageView;
        VkSampler shadowmapSampler;

        void update() override;
        void recordCommands(VkCommandBuffer commandBuffer) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
        void endRendering(VkCommandBuffer commandBuffer,uint32_t imageIndex) override;

        void loadNode(std::shared_ptr<Node>& rootNode);
        void createImagesList(std::shared_ptr<Node>& node);
        void createImagesIndex(std::shared_ptr<Node>& node);

    public:
        ShadowmapRenderer(const ShadowmapRenderer&) = delete;
        ShadowmapRenderer &operator=(const ShadowmapRenderer&) = delete;
        ShadowmapRenderer(const ShadowmapRenderer&&) = delete;
        ShadowmapRenderer &&operator=(const ShadowmapRenderer&&) = delete;
    };

}