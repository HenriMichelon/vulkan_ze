#include "z0/application.hpp"
#include "z0/texture.hpp"

namespace z0 {

    ImageTexture::ImageTexture(Viewport &_viewport, const std::string& appdir, const std::string& filename) {
        image = std::make_shared<Image>(_viewport, appdir, filename);
    }

}