#include "z0/nodes/spot_light.hpp"

namespace z0 {

    SpotLight::SpotLight(glm::vec3 lightDirection,
            float cutOffDegrees,
            float outerCutOffDegrees,
            float linear,
            float quadratic,
            float attenuation,
            glm::vec4 color,
            float specular,
            const std::string nodeName):
            OmniLight{linear, quadratic, attenuation, color, specular, nodeName},
            direction{lightDirection},
            fov{glm::radians(outerCutOffDegrees)},
            cutOff{glm::cos(glm::radians(cutOffDegrees))},
            outerCutOff{glm::cos(fov)}
    {
    }

    void SpotLight::setCutOff(float cutOffDegrees) {
        SpotLight::cutOff = glm::cos(glm::radians(cutOffDegrees));
    }

    void SpotLight::setOuterCutOff(float outerCutOffDegrees) {
        fov = glm::radians(outerCutOffDegrees);
        SpotLight::outerCutOff = glm::cos(fov);
    }

}