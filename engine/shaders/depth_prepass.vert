#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    mat4 projection;
    mat4 view;
} global;

struct ModelUniform  {
    mat4 matrix;
};

layout(set = 0, binding = 1) uniform ModelUniformArray  {
    ModelUniform transforms[1];
} models;

layout(location = 0) in vec3 position;

void main() {
    mat4 model = models.transforms[gl_InstanceIndex].matrix;
    vec4 globalPosition = model * vec4(position, 1.0);
    gl_Position = global.projection * global.view * globalPosition;
}