#version 450

#include "input_datas.glsl"

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec3 POSITION;

layout(location = 0) out vec4 ALBEDO;

void main() {
    vec4 color = vec4(surface.albedoColor.rgb, 1.0);
    if (surface.textureIndex != -1) {
        color = texture(texSampler[surface.textureIndex], UV);
    }
    vec3 diffuseLight = global.ambientLightColor.xyz * global.ambientLightColor.w;
    ALBEDO = vec4(diffuseLight * color.rgb, 1.0);
}