#include "z0/nodes/physics_body.hpp"
#include "z0/application.hpp"

#include <glm/gtx/quaternion.hpp>

#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace z0 {

    PhysicsBody::PhysicsBody(std::shared_ptr<Shape>& _shape, uint32_t layer, uint32_t mask, JPH::EActivation _activationMode, JPH::EMotionType _motionType, const std::string name):
        Node{name},
        bodyInterface{Application::_getBodyInterface()},
        shape{_shape},
        activationMode{_activationMode},
        motionType{_motionType},
        collisionLayer{layer},
        collisionMask{mask} {
        const JPH::BodyCreationSettings settings{
                shape->_getShape(),
                JPH::RVec3(0.0f, 0.0f, 0.0f),
                JPH::Quat::sIdentity(),
                motionType,
                collisionLayer << 4 | collisionMask
        };
        bodyId = bodyInterface.CreateAndAddBody(settings, activationMode);
        setPositionAndRotation();
        needPhysics = true;
    }

    PhysicsBody::~PhysicsBody() {
        //bodyInterface.DestroyBody(bodyId);
    }

    void PhysicsBody::_physicsUpdate() {
        updating = true;
        //auto position = bodyInterface.GetPosition(bodyId);
        //setPositionGlobal(glm::vec3{position.GetX(), position.GetY(), position.GetZ()});
        auto mat44 = bodyInterface.GetWorldTransform(bodyId);

        updating = false;
    }

    void PhysicsBody::setPositionAndRotation() {
        if (updating || (parent == nullptr)) return;
        auto position = getPositionGlobal();
        auto quat = glm::toQuat(glm::mat3(worldTransform));
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void PhysicsBody::updateTransform() {
        //Node::updateTransform();
        //setPositionAndRotation();
    }

    void PhysicsBody::updateTransform(const glm::mat4 &parentMatrix) {
        //Node::updateTransform(parentMatrix);
        //setPositionAndRotation();
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
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void PhysicsBody::setCollistionMask(uint32_t layer, bool value) {
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

}