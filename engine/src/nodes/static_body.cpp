#include "z0/nodes/static_body.hpp"

namespace z0 {

    StaticBody::StaticBody(const std::string name): PhysicsBody(JPH::EActivation::DontActivate, name) {

    }

}