#pragma once

#include "z0/vulkan/vulkan_image.hpp"
#include "z0/viewport.hpp"
#include "z0/resource.hpp"

namespace z0 {

    class Image: public Resource {
    public:
        Image(const std::filesystem::path& filename);

        uint32_t getWidth() const { return vulkanImage->getWidth(); };
        uint32_t getHeight() const { return vulkanImage->getHeight(); };
        glm::vec2 getSize() const { return glm::vec2{getWidth(), getHeight()}; };

    private:
        std::shared_ptr<VulkanImage> vulkanImage;

    public:
        VulkanImage& _getImage() { return *vulkanImage; }
    };

}