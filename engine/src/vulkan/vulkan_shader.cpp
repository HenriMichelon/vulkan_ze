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
            device{dev}, stage{_stage}, shaderName{_name}, stageFlags{_next_stage}, spirv{code} {
        shaderCreateInfo.sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
        shaderCreateInfo.pNext                  = nullptr;
        shaderCreateInfo.flags                  = 0;
        shaderCreateInfo.stage                  = stage;
        shaderCreateInfo.nextStage              = stageFlags;
        shaderCreateInfo.codeType               = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        shaderCreateInfo.codeSize               = spirv.size() * sizeof(spirv[0]);
        shaderCreateInfo.pCode                  = spirv.data();
        shaderCreateInfo.pName                  = "main";
        shaderCreateInfo.setLayoutCount         = 1;
        shaderCreateInfo.pSetLayouts            = pSetLayouts;
        shaderCreateInfo.pushConstantRangeCount = 1;
        shaderCreateInfo.pPushConstantRanges    = pPushConstantRange;
        shaderCreateInfo.pSpecializationInfo    = nullptr;
   }

    void VulkanShader::destroy() {
        if (shader != VK_NULL_HANDLE) {
            vkDestroyShaderEXT(device.getDevice(), shader, nullptr);
            shader = VK_NULL_HANDLE;
        }
    }

    VulkanShaderObject::VulkanShaderObject(VulkanDevice &dev): device{dev} {}

    void VulkanShaderObject::buildLinkedShaders(VulkanShader *vert, VulkanShader *frag)
    {
        VkShaderCreateInfoEXT shader_create_infos[2];
        if (vert == nullptr || frag == nullptr) {
            die("buildLinkedShaders failed with null vertex or fragment shader");
        }

        shader_create_infos[0] = vert->getShaderCreateInfo();
        shader_create_infos[1] = frag->getShaderCreateInfo();
        for (auto &shader_create : shader_create_infos){
            shader_create.flags |= VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
        }

        VkShaderEXT shaderEXTs[2];
        if (vkCreateShadersEXT(
             device.getDevice(),
             2,
             shader_create_infos,
             nullptr,
             shaderEXTs) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed\n");
        }
        vert->setShader(shaderEXTs[0]);
        frag->setShader(shaderEXTs[1]);
    }

    void VulkanShaderObject::buildShader(VulkanShader *shader) {
        VkShaderEXT shaderEXT;
        VkShaderCreateInfoEXT shaderCreateInfo = shader->getShaderCreateInfo();
        if (vkCreateShadersEXT(device.getDevice(), 1, &shaderCreateInfo, nullptr, &shaderEXT) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed");
        }
        shader->setShader(shaderEXT);
    }

    void VulkanShaderObject::bindShader(VkCommandBuffer commandBuffer, VulkanShader *shader) {
        vkCmdBindShadersEXT(commandBuffer, 1, shader->getStage(), shader->getShader());
    }

}