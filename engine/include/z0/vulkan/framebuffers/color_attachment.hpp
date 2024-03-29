#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class ColorAttachment: public BaseFrameBuffer {
    public:
        explicit ColorAttachment(VulkanDevice &dev);
        void createImagesResources() override;
    };

}