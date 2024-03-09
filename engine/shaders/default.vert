#version 450

#include "input_datas.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 UV;


void main() {
    vec4 globalPosition = model.matrix * vec4(position, 1.0);
    gl_Position = global.projection * global.view * globalPosition;
    UV = uv;
}