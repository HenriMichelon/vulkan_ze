#include "z0/vulkan/renderers/shadow_map.hpp"
#include "z0/log.hpp"
#include "z0/nodes/directional_light.hpp"

namespace z0 {

    ShadowMap::ShadowMap(VulkanDevice &dev, Light* spotLight) :
            BaseSharedImage{dev, dev.findImageTilingSupportedFormat(
                    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)},
            light(spotLight) {
         createImagesResources();
     }

    ShadowMap::~ShadowMap() {
        cleanupImagesResources();
    }

    glm::mat4 ShadowMap::getLightSpace() const {
        if (auto* directionalLight = dynamic_cast<DirectionalLight*>(light)) {
            // Light properties
            glm::vec3 lightDir = glm::normalize(directionalLight->getDirection());
            // Scene bounds (you should calculate these based on your scene)
            glm::vec3 sceneMin = glm::vec3(-10.0f, -10.0f, -10.0f);
            glm::vec3 sceneMax = glm::vec3(10.0f, 10.0f, 10.0f);
            // Calculate center of the scene
            glm::vec3 sceneCenter = (sceneMin + sceneMax) / 2.0f;
            // Set up the orthographic projection matrix
            float orthoWidth = glm::distance(sceneMin.x, sceneMax.x);
            float orthoHeight = glm::distance(sceneMin.y, sceneMax.y);
            float orthoDepth = glm::distance(sceneMin.z, sceneMax.z);
            glm::mat4 lightProjection = glm::ortho(-orthoWidth / 2, orthoWidth / 2,
                                                   -orthoHeight / 2, orthoHeight / 2,
                                                   zNear, orthoDepth);
            // Set up the view matrix for the light
            glm::mat4 lightView = glm::lookAt(sceneCenter - lightDir * (orthoDepth / 2.0f), // Position is scene center offset by light direction
                                              sceneCenter,                                // Looks at the center of the scene
                                              glm::vec3(0.0f, 1.0f, 0.0f));               // Up is in the Y direction
            // Combine to form the light's space matrix
            return lightProjection * lightView;
        } else if (auto* spotLight = dynamic_cast<SpotLight*>(light)) {
            return  glm::perspective(spotLight->getFov(), 1.0f, zNear, zFar) *
                    glm::lookAt(light->getPosition(), glm::vec3(0.0f), glm::vec3(0, 1,0));
        }
        return glm::mat4{};
    }

    void ShadowMap::createImagesResources() {
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // For shadow mapping we only need a depth attachment
        vulkanDevice.createImage(size, size,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory);
        imageView = vulkanDevice.createImageView(image,
                                                 format,
                                                 VK_IMAGE_ASPECT_DEPTH_BIT,
                                                 1);

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
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanDevice.getDevice(), sampler, nullptr);
            vkDestroyImageView(vulkanDevice.getDevice(), imageView, nullptr);
            vkDestroyImage(vulkanDevice.getDevice(), image, nullptr);
            vkFreeMemory(vulkanDevice.getDevice(), imageMemory, nullptr);
            sampler = VK_NULL_HANDLE;
            imageView = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }


}