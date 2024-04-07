#pragma once

#include "z0/resources/mesh.hpp"

namespace z0 {

    class MeshInstance: public Node {
    public:
        explicit MeshInstance(const std::string name = "MeshInstance"): Node{name} { }
        explicit MeshInstance(const std::shared_ptr<Mesh>& _mesh, const std::string name = "MeshInstance"): Node{name}, mesh{_mesh} {};

        void setMesh(const std::shared_ptr<Mesh>& _mesh) { mesh = _mesh; };
        std::shared_ptr<Mesh>& getMesh() { return mesh; }
        bool isValid() const { return mesh != nullptr; }
        //std::string toString() const override;

    private:
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Node> duplicateInstance() override;
    };

}