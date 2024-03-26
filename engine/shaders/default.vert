#version 450

#include "input_datas.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

layout (location = 0) out VertexOut vs_out;

void main() {
    vs_out.UV = uv;
    vs_out.POSITION = position;
    vs_out.GLOBAL_POSITION = model.matrix * vec4(position, 1.0);
    vs_out.NORMAL = normalize(mat3(transpose(inverse(model.matrix))) * normal);
    gl_Position = global.projection * global.view * vs_out.GLOBAL_POSITION;
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    vec3 T = normalize(vec3(model.matrix * vec4(tangent,   0.0)));
    vec3 N = normalize(vec3(model.matrix * vec4(normal,    0.0)));
    vec3 B = cross(N, T);
    vs_out.TBN = transpose(mat3(T, B, N));
    vs_out.TANGENT_POSITION  = vs_out.TBN * vs_out.GLOBAL_POSITION.xyz;
    vs_out.TANGENT_VIEW_DIRECTION = normalize((vs_out.TBN * global.cameraPosition) - vs_out.TANGENT_POSITION);
}