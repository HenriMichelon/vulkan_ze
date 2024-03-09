layout(set = 0, binding = 0) uniform GlobalUniformBufferObject  {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
} global;

layout(binding = 1) uniform sampler2D texSampler[100]; // put a limit into the default renderer

layout(set = 0, binding = 2) uniform SurfaceUniformBufferObject  {
    mat4 model;
    int textureIndex;
    vec4 albedoColor;
} surface;
