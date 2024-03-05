#include "z0/mesh.hpp"
#include "z0/application.hpp"

namespace z0 {

    Mesh::Mesh(z0::Viewport &_viewport, const std::string& filename): viewport(_viewport) {
        vulkanModel = VulkanModel::createModelFromFile(
            viewport._getDevice(),
            Application::getApp().getApplicationDirectory() + filename
        );
    }

}