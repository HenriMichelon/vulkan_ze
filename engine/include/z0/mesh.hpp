#pragma once

#include "object.hpp"

namespace z0 {

    class Mesh: public Object {
    public:
        static std::unique_ptr<Mesh> createFromFile(const std::string &filepath);

    protected:
        Mesh() {};
    };

}