layout(set = 0, binding = 0) uniform SurfaceUniformBufferObject  {
    mat4 model;
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    int textureIndex;
    vec4 albedoColor;
} surface;

layout(set = 0, binding = 1) uniform GlobalUniformBufferObject  {
    mat4 model;
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    int textureIndex;
    vec4 albedoColor;
} global;