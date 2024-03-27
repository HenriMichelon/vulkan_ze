#pragma once

#include "z0/object.hpp"

namespace z0 {

    struct Vertex {
        glm::vec3   position{};
        glm::vec3   normal{};
        alignas(16) glm::vec2 uv{};
        glm::vec4   tangent{};

        bool operator==(const Vertex&other) const {
            return position == other.position && normal == other.normal && uv == other.uv;
        }
    };

}