#include "z0/resources/shape.hpp"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace z0 {

    BoxShape::BoxShape(glm::vec3 sizes, std::string resName):
        Shape {new JPH::BoxShape(JPH::Vec3(sizes.x, sizes.y, sizes.z)), resName} {
    }

}