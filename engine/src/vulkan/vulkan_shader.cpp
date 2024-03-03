#include <utility>

#include "z0/vulkan/vulkan_shader.hpp"
#include "z0/log.hpp"

namespace z0 {

    // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
    VulkanShader::VulkanShader(VulkanDevice& dev,
                               VkShaderStageFlagBits _stage,
                               VkShaderStageFlags _next_stage,
                               std::string _name,
                               const std::vector<char> &code,
                               const VkDescriptorSetLayout *pSetLayouts,
                               const VkPushConstantRange *pPushConstantRange):
            device{dev}, stage{_stage}, shaderName{std::move(_name)}, stageFlags{_next_stage}, spirv{code} {
        shaderCreateInfo.sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
        shaderCreateInfo.pNext                  = nullptr;
        shaderCreateInfo.flags                  = 0;
        shaderCreateInfo.stage                  = stage;
        shaderCreateInfo.nextStage              = stageFlags;
        shaderCreateInfo.codeType               = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        shaderCreateInfo.codeSize               = spirv.size() * sizeof(spirv[0]);
        shaderCreateInfo.pCode                  = spirv.data();
        shaderCreateInfo.pName                  = "main";
        shaderCreateInfo.setLayoutCount         = 0;
        shaderCreateInfo.pSetLayouts            = pSetLayouts;
        shaderCreateInfo.pushConstantRangeCount = 0;
        shaderCreateInfo.pPushConstantRanges    = pPushConstantRange;
        shaderCreateInfo.pSpecializationInfo    = nullptr;
   }

    VulkanShader::~VulkanShader() {
        if (shader != VK_NULL_HANDLE) {
            vkDestroyShaderEXT(device.getDevice(), shader, nullptr);
            shader = VK_NULL_HANDLE;
        }
    }



}