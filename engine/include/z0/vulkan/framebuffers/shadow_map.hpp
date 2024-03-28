#pragma once

#include "base_frame_buffer.hpp"
#include "z0/nodes/spot_light.hpp"

namespace z0 {

    class ShadowMap: public BaseFrameBuffer {
    public:
        explicit ShadowMap(VulkanDevice &dev, Light* light);
        ~ShadowMap();

        // Keep depth range as small as possible
        // for better shadow map precision const
        const float zNear = .1f;
        const float zFar = 100.0f;

#if defined(__ANDROID__)
	    const uint32_t size{ 1024 };
#else
        const uint32_t size{ 2048 };
#endif

        glm::mat4 getLightSpace() const;
        glm::vec3 getLightPosition() const { return light->getPosition(); }
        const VkSampler& getSampler() const { return sampler; }

        void createImagesResources();
        void cleanupImagesResources();

    private:
        Light* light;
        VkSampler sampler;
    };

}