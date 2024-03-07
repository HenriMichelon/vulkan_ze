#pragma once

#include "z0/mesh.hpp"
#include "node.hpp"

namespace z0 {

    class MeshInstance: public Node {
    public:
        MeshInstance() {};
        MeshInstance(std::shared_ptr<Mesh> _mesh): mesh{_mesh} {};

        void setMesg(std::shared_ptr<Mesh> _mesh) { mesh = _mesh; };
        std::shared_ptr<Mesh> getMesh() const { return mesh; }

    private:
        std::shared_ptr<Mesh> mesh;
    };

}