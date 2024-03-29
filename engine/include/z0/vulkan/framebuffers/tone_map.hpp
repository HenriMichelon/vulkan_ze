#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class ToneMap: public BaseFrameBuffer {
    public:
        explicit ToneMap(VulkanDevice &dev);
        void createImagesResources() override;
        void cleanupImagesResources() override;
        const VkSampler& getSampler() const { return sampler; }
        VkDescriptorImageInfo imageInfo();
    private:
        VkSampler sampler{VK_NULL_HANDLE};
    };

}