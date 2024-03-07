#pragma once

#include "z0/object.hpp"

namespace z0 {

    class Texture: public Object {
    public:
        virtual uint32_t getWidth() = 0;
        virtual uint32_t getHeight() = 0;
        virtual glm::vec2 getSize() = 0;

    };


}