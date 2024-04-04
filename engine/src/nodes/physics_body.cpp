#include "z0/nodes/physics_body.hpp"
#include "z0/application.hpp"

#include <glm/gtx/quaternion.hpp>

#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace z0 {

    PhysicsBody::PhysicsBody(JPH::EActivation _activationMode, JPH::EMotionType _motionType, const std::string name):
        Node{name}, activationMode{_activationMode}, motionType{_motionType} {
        JPH::BodyCreationSettings settings{};
        auto& bodyInterface = Application::_getBodyInterface();
        bodyId = bodyInterface.CreateAndAddBody(settings, activationMode);
        bodyInterface.SetMotionType(bodyId, motionType, activationMode);
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    PhysicsBody::~PhysicsBody() {
        Application::_getBodyInterface().RemoveBody(bodyId);
    }

    void PhysicsBody::setPositionAndRotation() {
        auto position = getPositionGlobal();
        auto quat = glm::toQuat(glm::mat3(worldTransform));
        Application::_getBodyInterface().SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void PhysicsBody::updateTransform() {
        Node::updateTransform();
        setPositionAndRotation();
    }

    void PhysicsBody::updateTransform(const glm::mat4 &parentMatrix) {
        Node::updateTransform(parentMatrix);
        setPositionAndRotation();
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
        Application::_getBodyInterface().SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void PhysicsBody::setCollistionMask(uint32_t layer, bool value) {
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        Application::_getBodyInterface().SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

}