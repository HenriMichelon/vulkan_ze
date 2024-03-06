#version 450

layout(binding = 0) uniform UniformBufferObject  {
    mat4 model;
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    uint textureBinding;
} ubo;

layout(binding = 1) uniform sampler2D texSampler[2];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler[ubo.textureBinding], fragUV);
}