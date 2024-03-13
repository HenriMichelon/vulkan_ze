struct DirectionalLight {
    vec3 direction;
    vec4 color;
    float specular;
};

struct PointLight {
    vec3 position;
    vec4 color;
    float specular;
    float constant;
    float linear;
    float quadratic;
    bool isSpot;
    vec3 direction;
    float cutOff;
    float outerCutOff;
};

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject  {
    mat4 projection;
    mat4 view;
    vec4 ambient;
    vec3 cameraPosition;
    DirectionalLight directionalLight;
    bool haveDirectionalLight;
    int pointLightsCount;
    bool haveShadowMap;
    mat4 lightSpace;
    vec3 lightPos;
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

layout(set = 0, binding = 4) uniform PointLightArray {
    PointLight lights[1];
} pointLights;
