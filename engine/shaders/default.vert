#version 450

layout(binding = 0) uniform UniformBufferObject  {
    mat4 model;
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    uint textureBinding;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

void main() {
    vec4 globalPosition = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.projection * ubo.view * globalPosition;
    fragColor = inColor;
    fragUV = inUV;
}