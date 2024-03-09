#include "z0/nodes/mesh_instance.hpp"

namespace z0 {

    std::string MeshInstance::toString() const {
        std::string n = Node::toString();
        return n + (mesh != nullptr ? "[" + mesh->getName() + "]" : "[]");
    }

}