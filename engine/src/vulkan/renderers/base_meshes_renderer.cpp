#include "z0/vulkan/renderers/base_meshes_renderer.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/vulkan/vulkan_descriptors.hpp"
#include "z0/log.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace z0 {

    BaseMeshesRenderer::BaseMeshesRenderer(VulkanDevice &dev, std::string sDir) : BaseRenderer(dev, sDir)
    {
    }

    void BaseMeshesRenderer::cleanup() {
        depthBuffer.reset();
        modelsBuffers.clear();
        BaseRenderer::cleanup();
    }

    void BaseMeshesRenderer::setInitialState(VkCommandBuffer commandBuffer) {
        bindShaders(commandBuffer);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, vulkanDevice.getSamples());
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        setViewport(commandBuffer, vulkanDevice.getSwapChainExtent().width, vulkanDevice.getSwapChainExtent().height);
        std::vector<VkVertexInputBindingDescription2EXT> vertexBinding = VulkanModel::getBindingDescription();
        std::vector<VkVertexInputAttributeDescription2EXT> vertexAttribute = VulkanModel::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());
    }

}