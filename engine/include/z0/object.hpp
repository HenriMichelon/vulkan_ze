#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "z0/log.hpp"

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>

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