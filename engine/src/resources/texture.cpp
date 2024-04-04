#include "z0/application.hpp"
#include "z0/resources/texture.hpp"

namespace z0 {

    ImageTexture::ImageTexture(const std::filesystem::path& filename): Texture{filename.string()} {
        image = std::make_shared<Image>(filename);
    }

}