#pragma once

#include "z0/nodes/physics_body.hpp"

namespace z0 {

    class RigidBody: public PhysicsBody {
    public:
        explicit RigidBody(const std::string name = "RigidBody");
    };

}