#include "z0/mainloop.hpp"
#include "z0/texture.hpp"

namespace z0 {

    ImageTexture::ImageTexture(const std::filesystem::path& filename) {
        image = std::make_shared<Image>(filename);
    }

}