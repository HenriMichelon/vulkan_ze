#pragma once

#include "z0/image.hpp"

namespace z0 {

    class Texture: public Resource {
    public:
        Texture() = default;
        Texture(const std::shared_ptr<Image>& img): image{img} {};

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
        ImageTexture(const std::shared_ptr<Image>& img):Texture(img) {};
        ImageTexture(const std::filesystem::path& filename);
    };


}