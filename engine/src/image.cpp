#include "z0/application.hpp"
#include "z0/image.hpp"

namespace z0 {

    Image::Image(Viewport &_viewport, const std::string& appdir, const std::string& filename): viewport(_viewport) {
        vulkanImage = VulkanImage::createFromFile(
            viewport._getDevice(),
            appdir + "/" + filename
        );
    }

}