#pragma once

#include "z0/object.hpp"

namespace z0 {

    struct Vertex {
        glm::vec3 position{};
        glm::vec4 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};
        bool operator==(const Vertex&other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

}