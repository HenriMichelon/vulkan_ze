struct DirectionalLight {
    vec3 direction;
    vec4 color;
    vec4 ambient;
    vec3 diffuse;
    vec4 specular;
};

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject  {
    mat4 projection;
    mat4 view;
    vec3 cameraPosition;
    DirectionalLight directionalLight;
} global;

layout(binding = 1) uniform sampler2D texSampler[100]; // put a limit into the default renderer

layout(set = 0, binding = 2) uniform ModelUniformBufferObject  {
    mat4 matrix;
    mat4 normalMatrix;
} model;

layout(set = 0, binding = 3) uniform SurfaceUniformBufferObject  {
    int diffuseIndex;
    int specularIndex;
    vec4 albedoColor;
    float shininess;
} material;
