#include "z0/cubemap.hpp"
#include "z0/viewport.hpp"
#include "z0/application.hpp"

namespace z0 {

    Cubemap::Cubemap(const std::filesystem::path& filename, const std::string& fileext) {
        loadFromFile(filename, fileext);
    }

    void Cubemap::loadFromFile(const std::filesystem::path& filename, const std::string& fileext) {
        vulkanCubemap = VulkanCubemap::createFromFile(Application::getViewport()._getDevice(), (Application::getDirectory() / filename).string(), fileext);
        name = filename.string();
    }

}