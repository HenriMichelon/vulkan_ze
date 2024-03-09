#version 450

#include "ubo.glsl"

layout(binding = 1) uniform sampler2D texSampler[100]; // put a limit into the default renderer

layout(location = 1) in vec2 UV;

layout(location = 0) out vec4 ALBEDO;

void main() {
    if (ubo.textureIndex == -1) {
        ALBEDO = ubo.albedoColor;
    } else {
        ALBEDO = texture(texSampler[ubo.textureIndex], UV);
        ALBEDO.a = ubo.albedoColor.a;
    }
}