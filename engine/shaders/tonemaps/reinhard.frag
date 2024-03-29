#version 450

layout (location = 0) out vec4 COLOR;
layout (location = 0) in vec2 UV;
layout(set = 0, binding = 1) uniform sampler2D hdrBuffer;

void main()
{
    const float gamma = 1.0;
    vec3 hdrColor = texture(hdrBuffer, UV).rgb;
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));
    COLOR = vec4(mapped, 1.0);
}