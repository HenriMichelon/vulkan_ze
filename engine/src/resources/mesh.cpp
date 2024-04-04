#include "z0/resources/mesh.hpp"
#include "z0/application.hpp"

namespace z0 {

    std::shared_ptr<Material>& Mesh::getSurfaceMaterial(uint32_t surfaceIndex) {
        return surfaces[surfaceIndex]->material;
    }

    void Mesh::setSurfaceMaterial(uint32_t surfaceIndex, std::shared_ptr<Material>& material) {
        surfaces[surfaceIndex]->material = material;
    }

    void Mesh::_buildModel() {
        _model = std::make_shared<VulkanModel>(Application::getViewport()._getDevice(), vertices, indices);
    }

}