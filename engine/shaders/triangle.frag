#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragUV * 1.0);
    //outColor = vec4(fragColor * texture(texSampler, fragUV).rgb, 1.0);
}