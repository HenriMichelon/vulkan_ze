#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class DepthBuffer: public BaseFrameBuffer {
    public:
        explicit DepthBuffer(VulkanDevice &dev);
        void createImagesResources();
    };

}