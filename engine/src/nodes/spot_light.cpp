#include "z0/nodes/spot_light.hpp"

namespace z0 {

    SpotLight::SpotLight(glm::vec3 lightDirection,
            float _cutOff,
            float _outerCutOff,
            float linear,
            float quadratic,
            float attenuation,
            glm::vec4 color,
            float specular,
            const std::string nodeName):
            OmniLight{linear, quadratic, attenuation, color, specular, nodeName},
            cutOff{glm::cos(glm::radians(_cutOff))},
            outerCutOff{glm::cos(glm::radians(_outerCutOff))}
    {
    }

    void SpotLight::setCutOff(float cutOffDegrees) {
        SpotLight::cutOff = glm::cos(glm::radians(cutOffDegrees));
    }

    void SpotLight::setOuterCutOff(float outerCutOffDegrees) {
        SpotLight::outerCutOff = glm::cos(glm::radians(outerCutOffDegrees));
    }

}