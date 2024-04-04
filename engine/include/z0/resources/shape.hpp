#pragma once

#include "z0/resources/resource.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace z0 {

    class Shape : public Resource {
    public:
        bool isValid() override { return shape != nullptr; };

    protected:
        explicit Shape(JPH::Shape* _shape, std::string resName): Resource{resName}, shape{_shape} {};

    private:
        JPH::Shape* shape;

    public:
        JPH::Shape* _getShape() { return shape; }
    };

    class BoxShape : public Shape {
    public:
        explicit BoxShape(glm::vec3 sizes, std::string resName = "BoxShape");
    };

}