#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    /*std::string MeshInstance::toString() const {
        std::string n = Node::toString();
        return n + (mesh != nullptr ? "[" + mesh->getName() + "]" : "[]");
    }*/

    std::shared_ptr<Node> MeshInstance::duplicateInstance() {
        return std::make_shared<MeshInstance>(*this);
    }

}