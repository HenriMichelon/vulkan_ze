#version 450

layout (location = 0) in vec3 position;

layout (binding = 0) uniform UBO {
    mat4 depthMVP;
} ubo;

out gl_PerVertex  {
    vec4 gl_Position;
};

void main() {
    gl_Position =  ubo.depthMVP * vec4(position, 1.0);
}