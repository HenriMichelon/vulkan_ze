#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    // Rendering attachments
    class ColorAttachment: public BaseFrameBuffer {
    public:
        // If multisampled==true attachment will support multisampling *and* HDR
        explicit ColorAttachment(VulkanDevice &dev, bool multisampled);
        void createImagesResources() override;
    private:
        bool multisampled;
    };

}