#include "z0/nodes/static_body.hpp"

namespace z0 {

    StaticBody::StaticBody(std::shared_ptr<Shape> shape, uint32_t layer, uint32_t mask, const std::string name):
        PhysicsBody(shape, layer, mask, JPH::EActivation::DontActivate, JPH::EMotionType::Static, name) {

    }

}