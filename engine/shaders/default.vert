#version 450

layout(binding = 0) uniform UniformBufferObject  {
    mat4 model;
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    int textureIndex;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 COLOR;
layout(location = 1) out vec2 UV;

void main() {
    vec4 globalPosition = ubo.model * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * globalPosition;
    COLOR = color;
    UV = uv;
}