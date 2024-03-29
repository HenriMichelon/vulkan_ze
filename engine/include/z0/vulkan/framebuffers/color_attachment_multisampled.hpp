#pragma once

#include "base_frame_buffer.hpp"

namespace z0 {

    class ColorAttachmentMultisampled: public BaseFrameBuffer {
    public:
        // HDR tone mapping
        // Table 47. Mandatory format support : 16 - bit channels
        // https://www.khronos.org/registry/vulkan/specs/1.0/pdf/vkspec.pdf
        static const VkFormat renderFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        explicit ColorAttachmentMultisampled(VulkanDevice &dev);
        void createImagesResources();
    };

}