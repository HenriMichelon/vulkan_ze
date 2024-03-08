#include "z0/application.hpp"
#include "z0/image.hpp"

namespace z0 {

    Image::Image(const std::filesystem::path& filename) {
        vulkanImage = VulkanImage::createFromFile(
            Application::getViewport()._getDevice(),
            (Application::getDirectory() / filename).string()
        );
    }

}