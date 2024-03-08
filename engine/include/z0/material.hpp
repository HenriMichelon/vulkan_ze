#pragma once

#include "z0/color.hpp"
#include "z0/texture.hpp"

namespace z0 {

    class Material: public Resource {

    };

    class StandardMaterial: public Material {
    public:
        Color                       albedo_color{1.0f, 1.0f, 1.0f, 1.0f};
        std::shared_ptr<Texture>    albedo_texture;

        bool isValid() override { return true; }

    };

}