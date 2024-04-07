#include "z0/nodes/multi_mesh_instance.hpp"

namespace z0 {

    MultiMeshInstance::MultiMeshInstance(const std::shared_ptr<Mesh> mesh, const uint32_t instanceCount,
                                         const std::string name):
         Node{name} {
        multiMesh = std::make_shared<MultiMesh>(mesh, instanceCount, mesh->toString());
    }

    std::shared_ptr<Node> MultiMeshInstance::duplicateInstance() {
        return std::make_shared<MultiMeshInstance>(*this);
    }

}