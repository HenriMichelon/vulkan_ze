#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class Multisampled: public BaseFrameBuffer {
    public:
        explicit Multisampled(VulkanDevice &dev, VkFormat format);
        void createImagesResources();
    };

}