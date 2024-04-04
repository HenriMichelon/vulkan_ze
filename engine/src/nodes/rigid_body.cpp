#include "z0/nodes/rigid_body.hpp"

namespace z0 {

    RigidBody::RigidBody(const std::string name): PhysicsBody(JPH::EActivation::Activate, JPH::EMotionType::Dynamic, name) {

    }

}