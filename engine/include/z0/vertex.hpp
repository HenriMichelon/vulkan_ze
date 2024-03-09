#pragma once

#include "z0/object.hpp"

namespace z0 {

    struct Vertex {
        glm::vec3 position  {};
        glm::vec3 normal    { 1, 0, 0 };
        glm::vec2 uv        { 0, 0 };

        bool operator==(const Vertex&other) const {
            return position == other.position && normal == other.normal && uv == other.uv;
        }
    };

}