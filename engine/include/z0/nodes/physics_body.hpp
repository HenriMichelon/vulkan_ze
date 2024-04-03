#pragma once

#include "z0/nodes/node.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EActivation.h>

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

    protected:
        PhysicsBody(JPH::EActivation activationMode, const std::string name = "PhysicsBody");

    private:
        JPH::BodyID bodyId;
        uint32_t collisionLayer{1};
        uint32_t collisionMask{1};
    };

}