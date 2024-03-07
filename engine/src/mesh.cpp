#include "z0/mesh.hpp"
#include "z0/application.hpp"

namespace z0 {

    Mesh::Mesh(Viewport &_viewport, const std::string& filename): viewport(_viewport) {
        vulkanModel = VulkanModel::createFromFile(
                viewport._getDevice(),
                Application::getApp().getApplicationDirectory() + filename
        );
    }

}