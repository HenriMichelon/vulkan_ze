#version 450

#include "input_datas.glsl"

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 ALBEDO;

void main() {
    if (surface.textureIndex == -1) {
        ALBEDO = surface.albedoColor;
    } else {
        ALBEDO = texture(texSampler[surface.textureIndex], UV);
        ALBEDO.a = surface.albedoColor.a;
    }
}