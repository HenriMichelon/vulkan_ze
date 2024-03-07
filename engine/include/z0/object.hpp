#pragma once

#include "z0/log.hpp"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

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