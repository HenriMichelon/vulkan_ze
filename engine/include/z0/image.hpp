#pragma once

#include "z0/vulkan/vulkan_image.hpp"
#include "z0/viewport.hpp"

namespace z0 {

    class Image: public Object {
    public:
        Image(Viewport& viewport, const std::string& appdir, const std::string& filename);

        uint32_t getWidth() const { return vulkanImage->getWidth(); };
        uint32_t getHeight() const { return vulkanImage->getHeight(); };
        glm::vec2 getSize() const { return glm::vec2{getWidth(), getHeight()}; };

    private:
        Viewport &viewport;
        std::shared_ptr<VulkanImage> vulkanImage;

    public:
        VulkanImage& _getImage() { return *vulkanImage; }
    };

}