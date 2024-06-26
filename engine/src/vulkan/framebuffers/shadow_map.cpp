#include "z0/vulkan/framebuffers/shadow_map.hpp"
#include "z0/nodes/spot_light.hpp"
#include "z0/nodes/directional_light.hpp"
#include "z0/log.hpp"

namespace z0 {

    ShadowMap::ShadowMap(VulkanDevice &dev, Light* spotLight) : BaseFrameBuffer{dev}, light(spotLight) {
         createImagesResources();
     }

    glm::mat4 ShadowMap::getLightSpace() const {
        glm::vec3 lightPosition;
        glm::vec3 sceneCenter;
        glm::mat4 lightProjection;

        if (auto* directionalLight = dynamic_cast<DirectionalLight*>(light)) {
            auto lightDirection = glm::normalize(directionalLight->getDirection());
            // Scene bounds
            auto sceneMin = glm::vec3(-10.0f, -10.0f, -10.0f);
            auto sceneMax = glm::vec3(10.0f, 10.0f, 10.0f);
            // Set up the orthographic projection matrix
            auto orthoWidth = glm::distance(sceneMin.x, sceneMax.x);
            auto orthoHeight = glm::distance(sceneMin.y, sceneMax.y);
            auto orthoDepth = glm::distance(sceneMin.z, sceneMax.z);
            sceneCenter = (sceneMin + sceneMax) / 2.0f;
            lightPosition = sceneCenter - lightDirection * (orthoDepth / 2.0f); // Position is scene center offset by light direction
            lightProjection = glm::ortho(-orthoWidth / 2, orthoWidth / 2,
                                         -orthoHeight / 2, orthoHeight / 2,
                                         zNear, orthoDepth);
        } else if (auto* spotLight = dynamic_cast<SpotLight*>(light)) {
            auto lightDirection = glm::normalize(spotLight->getDirection());
            lightPosition = light->getPositionGlobal();
            sceneCenter = lightPosition + lightDirection;
            lightProjection = glm::perspective(spotLight->getFov(), vulkanDevice.getAspectRatio(), zNear, zFar);
        } else {
            return glm::mat4{};
        }
        // Combine the projecttion and view matrix to form the light's space matrix
        return lightProjection * glm::lookAt(lightPosition, sceneCenter, AXIS_UP);
    }

    // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
    void ShadowMap::createImagesResources() {
        // For shadow mapping we only need a depth attachment
        auto format = vulkanDevice.findImageTilingSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        createImage(size,
                    size,
                    format,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);

        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        VkFilter shadowmap_filter = vulkanDevice.formatIsFilterable( format, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        VkSamplerCreateInfo samplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = shadowmap_filter,
            .minFilter = shadowmap_filter,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = samplerCreateInfo.addressModeU,
            .addressModeW = samplerCreateInfo.addressModeU,
            .mipLodBias = 0.0f,
            .maxAnisotropy = 1.0f,
            .minLod = 0.0f,
            .maxLod = 1.0f,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
        };
        if (vkCreateSampler(vulkanDevice.getDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create shadowmap sampler!");
        }

    }

    void ShadowMap::cleanupImagesResources() {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanDevice.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        BaseFrameBuffer::cleanupImagesResources();
    }

}