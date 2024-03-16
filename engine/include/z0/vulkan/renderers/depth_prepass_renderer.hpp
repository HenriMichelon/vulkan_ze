#pragma once

#include "z0/vulkan/renderers/base_renderer.hpp"
#include "z0/vulkan/renderers/depth_buffer.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {

    class DepthPrepassRenderer: public BaseRenderer {
    public:
        struct GlobalUniformBufferObject {
            glm::mat4 projection{1.0f};
            glm::mat4 view{1.0f};
        };
        struct ModelUniformBufferObject {
            glm::mat4 matrix;
        };

        DepthPrepassRenderer(VulkanDevice& device, const std::string& shaderDirectory);

        void loadScene(std::shared_ptr<DepthBuffer>& buffer,  Camera* camera, std::vector<MeshInstance*>& meshes);
        void cleanup() override;

    private:
        Camera* camera;
        std::shared_ptr<DepthBuffer> depthBuffer;
        std::vector<MeshInstance*> meshes {};
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, VkImage swapChainImage) override;

    };

}