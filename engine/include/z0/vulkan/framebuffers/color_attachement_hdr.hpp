#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class ColorAttachementHDR: public BaseFrameBuffer {
    public:
        explicit ColorAttachementHDR(VulkanDevice &dev);
        void createImagesResources() override;
        void cleanupImagesResources() override;
        VkDescriptorImageInfo imageInfo();
    private:
        VkSampler sampler{VK_NULL_HANDLE};
    };

}