#pragma once

#include "viewport.hpp"
#include "z0/vulkan/vulkan_image.hpp"

namespace z0 {

    class Image: public Object {
    public:
        Image(Viewport& viewport, const std::string& appdir, const std::string& filename);

        uint32_t getWidth() const;
        uint32_t getHeight() const;
        glm::vec2 getSize() const;

    private:
        Viewport &viewport;
        std::shared_ptr<VulkanImage> vulkanImage;

    public:
        VulkanImage& _getImage() { return *vulkanImage; }
    };

}