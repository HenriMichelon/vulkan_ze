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
    public:
        explicit Material(std::string name = ""): Resource(name) {}
    };

    class StandardMaterial: public Material {
    public:
        Color                       albedoColor {0.8f, 0.3f, 0.5f, 1.0f };
        std::shared_ptr<Texture>    albedoTexture;
        CullMode                    cullMode { CULLMODE_BACK };

        explicit StandardMaterial(std::string name = ""): Material(name) {}
        bool isValid() override { return true; }
    };

}