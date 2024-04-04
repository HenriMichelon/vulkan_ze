#pragma once

#include "z0/nodes/physics_body.hpp"

namespace z0 {

    class RigidBody: public PhysicsBody {
    public:
        explicit RigidBody(std::shared_ptr<Shape> shape,
                           uint32_t layer=1,
                           uint32_t mask=1,
                           const std::string name = "RigidBody");
        virtual ~RigidBody() = default;

        void setBounce(float value);
    };

}