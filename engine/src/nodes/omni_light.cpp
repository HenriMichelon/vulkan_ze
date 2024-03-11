#include "z0/nodes/omni_light.hpp"

namespace z0 {

    OmniLight::OmniLight(float _linear,
                         float _quadratic,
                         float _attenuation,
                         glm::vec4 color, float specular, const std::string nodeName):
            Light{color, specular, nodeName},
            linear{_linear}, quadratic{_quadratic}, attenuation{_attenuation}
    {
    }

}