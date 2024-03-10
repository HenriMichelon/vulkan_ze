layout(set = 0, binding = 0) uniform GlobalUniformBufferObject  {
    mat4 projection;
    mat4 view;
    vec3 directionalLightDirection;
    vec4 directionalLightColor;
    vec4 ambientLightColor;
} global;

layout(binding = 1) uniform sampler2D texSampler[100]; // put a limit into the default renderer

layout(set = 0, binding = 2) uniform ModelUniformBufferObject  {
    mat4 matrix;
    mat4 normalMatrix;
} model;

layout(set = 0, binding = 3) uniform SurfaceUniformBufferObject  {
    int textureIndex;
    vec4 albedoColor;
} surface;
