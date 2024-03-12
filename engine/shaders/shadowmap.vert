#version 450

layout (location = 0) in vec3 position;

layout (binding = 0) uniform UBO {
    mat4 depthMVP;
    mat4 model;
} ubo;

out gl_PerVertex  {
    vec4 gl_Position;
};

void main() {
    gl_Position =  ubo.depthMVP * ubo.model * vec4(position, 1.0);
}