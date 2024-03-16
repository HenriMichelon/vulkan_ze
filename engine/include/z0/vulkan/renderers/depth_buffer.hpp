#pragma once

#include "z0/vulkan/renderers/base_shared_image.hpp"

namespace z0 {

    class DepthBuffer: public BaseSharedImage {
    public:
        explicit DepthBuffer(VulkanDevice &dev);
        ~DepthBuffer();

        void createImagesResources();
        void cleanupImagesResources();
    };

}