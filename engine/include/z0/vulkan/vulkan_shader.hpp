#pragma once

#include "z0/vulkan/vulkan_device.hpp"

namespace z0 {

    class VulkanShader {
    public:
        VulkanShader(VulkanDevice                &device,
                     VkShaderStageFlagBits       stage,
                     VkShaderStageFlags          next_stage,
                     std::string                 name,
                     const std::vector<char>     &code,
                     const VkDescriptorSetLayout *pSetLayouts,
                     const VkPushConstantRange   *pPushConstantRange);
        ~VulkanShader();

        [[nodiscard]] VkShaderCreateInfoEXT getShaderCreateInfo() const { return shaderCreateInfo; };
        [[nodiscard]] VkShaderStageFlagBits* getStage() { return &stage; };
        [[nodiscard]] VkShaderEXT* getShader() { return &shader; };

        void setShader(VkShaderEXT _shader) { shader = _shader; };

    private:
        VulkanDevice& device;
        VkShaderStageFlagBits stage;
        VkShaderStageFlags    stageFlags;
        VkShaderEXT           shader      = VK_NULL_HANDLE;
        std::string           shaderName;
        VkShaderCreateInfoEXT shaderCreateInfo;
        std::vector<char> spirv;
    };

}