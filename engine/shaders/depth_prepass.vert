#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 projection;
    mat4 view;
} global;

layout(set = 0, binding = 2) uniform ModelUniformBufferObject {
    mat4 matrix;
} model;

layout(location = 0) in vec3 position;

void main() {
    vec4 globalPosition = model.matrix * vec4(position, 1.0);
    gl_Position = global.projection * global.view * globalPosition;
}