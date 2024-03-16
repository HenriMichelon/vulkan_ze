#version 450

#include "input_datas.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 UV;
layout(location = 1) out vec3 NORMAL;
layout(location = 2) out vec3 POSITION;
layout(location = 4) out vec4 SHADOW_COORD;
layout(location = 5) out vec3 LIGHT_DIR;

void main() {
    vec4 globalPosition = model.matrix * vec4(position, 1.0);
    gl_Position = global.projection * global.view * globalPosition;
    UV = uv;
    NORMAL = normalize(mat3(model.normalMatrix) * normal);
    POSITION = globalPosition.xyz;

    if (global.haveShadowMap)
    {
        SHADOW_COORD = global.lightSpace * globalPosition;
        LIGHT_DIR = normalize(global.lightPos - position);
    }
}