#pragma once

#include "z0/vulkan/renderers//base_shared_image.hpp"
#include "z0/nodes/spot_light.hpp"

namespace z0 {

    class ShadowMap: public BaseSharedImage {
    public:
        explicit ShadowMap(VulkanDevice &dev, SpotLight* spotLight);
        ~ShadowMap();

#if defined(__ANDROID__)
	    const uint32_t size{ 1024 };
#else
        const uint32_t size{ 2048 };
#endif

        SpotLight* getLight() { return light; }
        const VkSampler& getSampler() const { return sampler; }

        void createImagesResources();
        void cleanupImagesResources();

    private:
        SpotLight* light;
        VkSampler sampler;
    };

}