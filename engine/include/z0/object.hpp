#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "z0/log.hpp"

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace z0 {

    class Object {
    public:
        Object();

        std::string getClassName() const { return std::string{class_name}; }
        virtual std::string toString() const { return getClassName(); };

    private:
        const char* class_name;
    };

}