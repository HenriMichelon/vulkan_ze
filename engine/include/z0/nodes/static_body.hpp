#pragma once

#include "z0/nodes/physics_body.hpp"

namespace z0 {

    class StaticBody: public PhysicsBody {
    public:
        explicit StaticBody(const std::string name = "StaticBody");
    };

}