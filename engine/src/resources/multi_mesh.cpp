#include "z0/resources/multi_mesh.hpp"

namespace z0 {

    MultiMesh::MultiMesh(const std::shared_ptr<Mesh> _mesh, uint32_t count, const std::string& meshName):
            Resource{meshName}, mesh{_mesh} {
        setInstanceCount(count);
    }

    void MultiMesh::setInstanceCount(uint32_t count) {
        transforms.resize(count);
    }

    uint32_t MultiMesh::getInstanceCount() const {
        return transforms.size();
    }

    void MultiMesh::setInstanceTransform(uint32_t instance, glm::mat4 transform) {
        if (instance < getInstanceCount()) transforms[instance] = transform;
    }

    glm::mat4 MultiMesh::getInstanceTransform(uint32_t instance) {
        return transforms[instance];
    }

}