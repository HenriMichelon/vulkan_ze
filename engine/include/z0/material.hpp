#pragma once

#include "z0/color.hpp"
#include "z0/texture.hpp"

namespace z0 {

    enum CullMode {
        CULLMODE_DISABLED   = 0,
        CULLMODE_BACK       = 1,
        CULLMODE_FRONT      = 2,
    };

    class Material: public Resource {

    };

    class StandardMaterial: public Material {
    public:
        Color                       albedoColor {1.0f, 1.0f, 1.0f, 1.0f};
        std::shared_ptr<Texture>    albedoTexture;
        CullMode                    cullMode { CULLMODE_BACK };

        bool isValid() override { return true; }

    };

}