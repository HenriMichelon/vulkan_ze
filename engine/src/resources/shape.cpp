#include "z0/resources/shape.hpp"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace z0 {

    BoxShape::BoxShape(glm::vec3 sizes, std::string resName):
        Shape {new JPH::BoxShape(JPH::Vec3(sizes.x/2, sizes.y/2, sizes.z/2)), resName} {
    }

}