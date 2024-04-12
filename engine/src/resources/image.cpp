#include "z0/resources/image.hpp"
#include "z0/application.hpp"
#include "z0/viewport.hpp"

namespace z0 {

    Image::Image(const std::filesystem::path& filename): Resource{filename.string()} {
        loadFromFile(filename);
    }

    void Image::loadFromFile(const std::filesystem::path& filename) {
        vulkanImage = VulkanImage::createFromFile(
                Application::getViewport()._getDevice(),
                (Application::getDirectory() / filename).string()
        );
    }

}