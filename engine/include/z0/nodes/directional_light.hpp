#pragma once

#include "z0/nodes/light.hpp"

namespace z0 {

    class DirectionalLight: public Light {
    public:
        explicit DirectionalLight(const std::string name): Light{name} {};
        DirectionalLight(glm::vec3 lightDirection,
                         glm::vec4 color,
                         float specular = 1.0f,
                         const std::string nodeName = ""):
                Light{color, specular, nodeName}, direction{glm::normalize(lightDirection)}  {}

        glm::vec3& getDirection() { return direction; }
        void setDirection(glm::vec3 lightDirection) { direction = lightDirection; }

    private:
        glm::vec3 direction = glm::vec3{0.0f, .5f, 1.0f};
    };

}