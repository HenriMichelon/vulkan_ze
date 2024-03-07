#include "z0/application.hpp"
#include "z0/texture.hpp"

namespace z0 {

    ImageTexture::ImageTexture(const std::string& filename) {
        image = std::make_shared<Image>(filename);
    }

}