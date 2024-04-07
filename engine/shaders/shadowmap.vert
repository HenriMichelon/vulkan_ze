#version 450

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;
layout (location = 0) out vec2 UV;

layout (binding = 0) uniform GlobalUBO {
    mat4 lightSpace;
} global;

struct ModelUniform  {
    mat4 matrix;
};

layout(set = 0, binding = 1) uniform ModelUniformArray  {
    ModelUniform transforms[1];
} models;

void main() {
    mat4 model = models.transforms[gl_InstanceIndex].matrix;
    vec4 globalPosition = model * vec4(position, 1.0);
    gl_Position = global.lightSpace * globalPosition;
    UV = uv;
}