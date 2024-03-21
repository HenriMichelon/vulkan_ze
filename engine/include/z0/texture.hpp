#pragma once

#include "z0/image.hpp"

namespace z0 {

    class Texture: public Resource {
    public:
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual glm::vec2 getSize() const { return glm::vec2{getWidth(), getHeight()}; };

    };

    class ImageTexture: public Texture {
    public:
        ImageTexture(const std::shared_ptr<Image>& img): image(img) {};
        ImageTexture(const std::filesystem::path& filename);

        bool isValid() override { return image != nullptr; }

        Image& getImage() { return *image; }
        uint32_t getWidth() const override { return image->getWidth(); };
        uint32_t getHeight() const override { return image->getHeight(); };

    protected:
        std::shared_ptr<Image> image {nullptr};
    };


}