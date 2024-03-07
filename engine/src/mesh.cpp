#include "z0/mesh.hpp"
#include "z0/application.hpp"

namespace z0 {

    Mesh::Mesh(Viewport &_viewport, const std::string& appdir,  const std::string& filename,  std::shared_ptr<Texture> _texture):
    viewport(_viewport), texture(_texture) {
        vulkanModel = VulkanModel::createFromFile(
                viewport._getDevice(),
                appdir + "/" + filename
        );
    }

}