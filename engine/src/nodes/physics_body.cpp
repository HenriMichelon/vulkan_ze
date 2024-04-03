#include "z0/nodes/physics_body.hpp"
#include "z0/application.hpp"

#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace z0 {

    PhysicsBody::PhysicsBody(JPH::EActivation activationMode, const std::string name): Node{name} {
        JPH::BodyCreationSettings settings{};
        bodyId = Application::_getBodyInterface().CreateAndAddBody(settings, activationMode);
    }

    PhysicsBody::~PhysicsBody() {
        Application::_getBodyInterface().RemoveBody(bodyId);
    }

    bool PhysicsBody::haveCollisionLayer(uint32_t layer) const {
        return collisionLayer & layer;
    }

    bool PhysicsBody::haveCollisionMask(uint32_t layer) const {
        return collisionMask & layer;
    }

    void PhysicsBody::setCollistionLayer(uint32_t layer, bool value) {
        if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
    }

    void PhysicsBody::setCollistionMask(uint32_t layer, bool value) {
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
    }

}