#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class ColorAttachement: public BaseFrameBuffer {
    public:
        explicit ColorAttachement(VulkanDevice &dev);
        void createImagesResources() override;
    };

}