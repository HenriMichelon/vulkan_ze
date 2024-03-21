#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject  {
    mat4 projection;
    mat4 view;
} global;

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 UV;

void main() {
    gl_Position = global.projection * global.view * vec4(position, 1.0);
    UV = position;
}