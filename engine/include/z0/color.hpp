#pragma once

#include "z0/object.hpp"

namespace z0 {

    struct Color {
        Color() = default;
        Color(glm::vec4 c) { color = c; }
        Color(glm::vec3 c) { color = glm::vec4(c, 1.0); }
        Color(float r, float g, float b) { color = glm::vec4(r, g, b, 1.0f); }
        Color(float r, float g, float b, float a) { color = glm::vec4(r, g, b, a); }

        glm::vec4 color;
    };

}