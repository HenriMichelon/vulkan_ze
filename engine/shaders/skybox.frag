#version 450

layout (location = 0) in vec3 UV;

layout (location = 0) out vec4 COLOR;

void main() {
    COLOR = vec4(UV.x, UV.y, UV.z, 1.0);
}