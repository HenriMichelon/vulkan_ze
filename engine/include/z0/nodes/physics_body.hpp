#pragma once

#include "z0/nodes/node.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/MotionType.h>

namespace z0 {

    class PhysicsBody: public Node {
    public:
        virtual ~PhysicsBody();

        uint32_t getCollisionLayer() const { return collisionLayer; }
        uint32_t getCollistionMask() const { return collisionMask; }
        bool haveCollisionLayer(uint32_t layer) const;
        bool haveCollisionMask(uint32_t layer) const;
        void setCollistionLayer(uint32_t layer, bool value);
        void setCollistionMask(uint32_t layer, bool value);

        void updateTransform() override;
        void updateTransform(const glm::mat4& parentMatrix) override;

    protected:
        PhysicsBody(JPH::EActivation activationMode, JPH::EMotionType motionType, const std::string name = "PhysicsBody");

    private:
        JPH::BodyID bodyId;
        JPH::EActivation activationMode;
        JPH::EMotionType motionType;
        uint32_t collisionLayer{1};
        uint32_t collisionMask{1};

        void setPositionAndRotation();
    };

}