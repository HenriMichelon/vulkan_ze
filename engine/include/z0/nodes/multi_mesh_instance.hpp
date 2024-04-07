#pragma once

#include "z0/resources/multi_mesh.hpp"

namespace z0 {

    class MultiMeshInstance: public Node {
    public:
        explicit MultiMeshInstance(const std::string name = "MultiMeshInstance"): Node{name} { }
        explicit MultiMeshInstance(const std::shared_ptr<Mesh> mesh, const uint32_t instanceCount, const std::string name = "MeshInstance");

        bool isValid() const { return multiMesh != nullptr; }
        std::shared_ptr<Node> duplicateInstance();

    private:
        std::shared_ptr<MultiMesh> multiMesh;
    };

}