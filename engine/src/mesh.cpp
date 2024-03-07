#include "z0/mesh.hpp"
#include "z0/application.hpp"

namespace z0 {

    Mesh::Mesh(const std::string& filename,  std::shared_ptr<Texture> _texture): texture(_texture) {
        vulkanModel = VulkanModel::createFromFile(
                Application::getViewport()._getDevice(),
                Application::getDirectory() + "/" + filename
        );
    }

}