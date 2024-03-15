#version 450
#include "tools.glsl"
// https://learnopengl.com/Advanced-OpenGL/Depth-testing

layout(location = 0) out vec4 COLOR;


void main() {
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    COLOR = vec4(vec3(depth), 1.0);
    //COLOR = vec4(gl_FragCoord.z, 0.0, 0.0, 1.0);
}