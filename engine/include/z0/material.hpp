#pragma once

#include "z0/color.hpp"
#include "z0/texture.hpp"

namespace z0 {

    enum CullMode {
        CULLMODE_DISABLED   = 0,
        CULLMODE_BACK       = 1,
        CULLMODE_FRONT      = 2,
    };

    enum Transparency {
        TRANSPARENCY_DISABLED         = 0,
        TRANSPARENCY_ALPHA            = 1, // alpha only
        TRANSPARENCY_SCISSOR          = 2, // scissor onmy
        TRANSPARENCY_SCISSOR_ALPHA    = 3, // scissor then alpha
    };

    class Material: public Resource {
    public:
        explicit Material(std::string name = ""): Resource(name) {}
    };

    class StandardMaterial: public Material {
    public:
        Color                       albedoColor {0.8f, 0.3f, 0.5f, 1.0f };
        std::shared_ptr<Texture>    albedoTexture;
        std::shared_ptr<Texture>    specularTexture;
        CullMode                    cullMode { CULLMODE_BACK };
        Transparency                transparency { TRANSPARENCY_DISABLED };
        float                       alphaScissor { 0.1 };

        explicit StandardMaterial(std::string name = ""): Material(name) {}
        bool isValid() override { return true; }
    };

}