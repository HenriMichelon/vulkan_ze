#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    // Rendering attachment or resolved offscreen depth buffer
    class DepthBuffer: public BaseFrameBuffer {
    public:
        explicit DepthBuffer(VulkanDevice &dev, bool multisampled);
        void createImagesResources();
    private:
        bool multisampled;
    };

}