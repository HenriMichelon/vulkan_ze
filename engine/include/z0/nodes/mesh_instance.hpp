#pragma once

#include "z0/mesh.hpp"

namespace z0 {

    class MeshInstance: public Node {
    public:
        MeshInstance() = default;
        MeshInstance(const std::shared_ptr<Mesh>& _mesh): mesh{_mesh} {};

        void setMesh(const std::shared_ptr<Mesh>& _mesh) { mesh = _mesh; };
        std::shared_ptr<Mesh> getMesh() const { return mesh; }
        bool isValid() const { return mesh != nullptr; }

    private:
        std::shared_ptr<Mesh> mesh;
    };

}