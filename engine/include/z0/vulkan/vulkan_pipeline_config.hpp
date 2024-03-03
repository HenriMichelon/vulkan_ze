#pragma once

#include "z0/vulkan/vulkan_model.hpp"

namespace z0 {
    class VulkanPipelineConfig {
    public:
        VulkanPipelineConfig(bool enableAlphaBlending = false);

        // In initialization order
        std::vector<VkDynamicState> dynamicStates{};
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        //std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        //std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        VkPipelineViewportStateCreateInfo viewportInfo{};
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

        VulkanPipelineConfig(const VulkanPipelineConfig&) = delete;
        VulkanPipelineConfig& operator=(const VulkanPipelineConfig&) = delete;
    };
}