#version 450

layout(location = 0) in vec3 position;

layout (binding = 0) uniform GlobalUBO {
    mat4 lightSpace;
} global;

layout (binding = 1) uniform ModelUBO {
    mat4 matrix;
} model;

void main() {
    gl_Position = global.lightSpace *  model.matrix * vec4(position, 1.0);
}