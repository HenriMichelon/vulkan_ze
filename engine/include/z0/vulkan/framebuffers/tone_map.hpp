#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class ToneMap: public BaseFrameBuffer {
    public:
        explicit ToneMap(VulkanDevice &dev, VkFormat format);
        void createImagesResources();
    };

}