#pragma once

#include "z0/vulkan/renderers/base_renderpass.hpp"
#include "z0/vulkan/framebuffers/depth_buffer.hpp"
#include "z0/nodes/multi_mesh_instance.hpp"
#include "z0/nodes/mesh_instance.hpp"
#include "z0/nodes/camera.hpp"

namespace z0 {


    class BaseMeshesRenderer: public BaseRenderpass, public VulkanRenderer {
    public:
        void cleanup() override;

        std::shared_ptr<DepthBuffer>& getDepthBuffer() { return depthBuffer; }

    protected:
        std::shared_ptr<Camera> currentCamera {nullptr};
        std::vector<std::shared_ptr<MeshInstance>> meshes {};
        std::shared_ptr<DepthBuffer> depthBuffer;
        std::vector<std::unique_ptr<VulkanBuffer>> modelsBuffers{MAX_FRAMES_IN_FLIGHT};

        BaseMeshesRenderer(VulkanDevice& device, std::string shaderDirectory);

        void setInitialState(VkCommandBuffer commandBuffer);

    public:
        BaseMeshesRenderer(const BaseMeshesRenderer&) = delete;
        BaseMeshesRenderer &operator=(const BaseMeshesRenderer&) = delete;
        BaseMeshesRenderer(const BaseMeshesRenderer&&) = delete;
        BaseMeshesRenderer &&operator=(const BaseMeshesRenderer&&) = delete;
    };

}