#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTC_constants
#include <glm/glm.hpp>

#include <memory>

namespace z0 {

    class Object {
    public:
        virtual std::string toString() const { return "??"; };
    };

}