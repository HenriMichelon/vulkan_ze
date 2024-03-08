#pragma once

#include "z0/image.hpp"

namespace z0 {

    class Texture: public Resource {
    public:
        bool isValid() override { return image != nullptr; }

        Image& getImage() { return *image; }
        uint32_t getWidth() const { return image->getWidth(); };
        uint32_t getHeight() const { return image->getHeight(); };
        glm::vec2 getSize() const { return glm::vec2{getWidth(), getHeight()}; };

    protected:
        std::shared_ptr<Image> image {nullptr};
    };

    class ImageTexture: public Texture {
    public:
        ImageTexture() = default;
        ImageTexture(const std::filesystem::path& filename);
    };


}