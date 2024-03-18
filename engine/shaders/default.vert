#version 450

#include "input_datas.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 UV;
layout(location = 1) out vec3 NORMAL;
layout(location = 2) out vec4 GLOBAL_POSITION;
layout(location = 3) out vec3 POSITION;

void main() {
    vec4 globalPosition = model.matrix * vec4(position, 1.0);
    gl_Position = global.projection * global.view * globalPosition;

    UV = uv;
    NORMAL = normalize(mat3(model.normalMatrix) * normal);
    GLOBAL_POSITION = globalPosition;
    POSITION = position;
}