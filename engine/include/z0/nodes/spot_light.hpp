#pragma once

#include "z0/nodes/omni_light.hpp"

namespace z0 {

    class SpotLight: public OmniLight {
    public:
        explicit SpotLight(const std::string name = "SpotLight"): OmniLight{name} {};
        explicit SpotLight(glm::vec3 lightDirection,
                           float cutOff,
                           float outerCutOff,
                           float linear,
                           float quadratic,
                           float attenuation = 1.0f,
                           glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                           float specular = 1.0f,
                           const std::string nodeName = "SpotLight");

        glm::vec3& getDirection() { return direction; }
        void setDirection(glm::vec3 lightDirection) { direction = lightDirection; }
        void setCutOff(float cutOffDegrees);
        float getCutOff() const {return cutOff;}
        void setOuterCutOff(float outerCutOffDegrees);
        float getOuterCutOff() const {return outerCutOff;}

    private:
        glm::vec3 direction{0.0f, 0.0f, 1.0f};
        float cutOff { glm::cos(glm::radians(10.f)) };
        float outerCutOff { glm::cos(glm::radians(15.f)) };
    };

}