#pragma once

#include "z0/nodes/light.hpp"

namespace z0 {

    class DirectionalLight: public Light {
    public:
        explicit DirectionalLight(const std::string name = "DirectionalLight"): Light{name} {};
        explicit DirectionalLight(glm::vec3 lightDirection,
                                  glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                                  float specular = 1.0f,
                                  const std::string nodeName = "DirectionalLight"):
                Light{color, specular, nodeName},
                direction{glm::normalize(lightDirection)}  {}
        virtual ~DirectionalLight() {};

        glm::vec3& getDirection() { return direction; }
        void setDirection(glm::vec3 lightDirection) { direction = lightDirection; }

    private:
        glm::vec3 direction{0.0f, .5f, 1.0f};
    };

}