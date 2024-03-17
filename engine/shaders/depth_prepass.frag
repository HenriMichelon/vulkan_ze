#version 450

layout(set = 0, binding = 3) uniform SurfaceUniformBufferObject  {
    bool transparency;
    int diffuseIndex;
    vec4 albedoColor;
} material;

layout(binding = 1) uniform sampler2D texSampler[100]; // put a limit into the default renderer

layout (location = 0) in vec2 UV;

layout (location = 0) out vec4 COLOR;

void main() {
    vec4 color = material.albedoColor;
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], UV);
    }
    if (color.a < 0.99) discard;

    COLOR = color;
}