#pragma once

#include "z0/vulkan/vulkan_image.hpp"
#include "z0/viewport.hpp"
#include "z0/resources/resource.hpp"

namespace z0 {

    class Image: public Resource {
    public:
        Image(std::string name = ""): Resource(name) {}
        explicit Image(const std::shared_ptr<VulkanImage>& image, std::string name = ""): Resource{name}, vulkanImage{image} {};
        explicit Image(const std::filesystem::path& filename);

        bool isValid() override { return vulkanImage != nullptr; }
        uint32_t getWidth() const { return vulkanImage->getWidth(); };
        uint32_t getHeight() const { return vulkanImage->getHeight(); };
        glm::vec2 getSize() const { return glm::vec2{getWidth(), getHeight()}; };

        void loadFromFile(const std::filesystem::path& filename);

    private:
        std::shared_ptr<VulkanImage> vulkanImage {nullptr};

    public:
        std::shared_ptr<VulkanImage>& _getImage() { return vulkanImage; }
    };

}