#version 450
#include "tools.glsl"

layout (location = 0) out vec4 COLOR;
layout (location = 0) in vec2 UV;
layout (binding = 5) uniform sampler2D shadowMap;
//layout(binding = 1) uniform sampler2D shadowMap[100]; // put a limit into the default renderer

void main()
{
	float depth = texture(shadowMap, UV).r;
	COLOR = vec4(vec3(LinearizeDepth(depth)), 1.0);
	//COLOR = texture(shadowMap[1], UV);
}