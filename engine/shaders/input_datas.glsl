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

struct ShadowMap {
    mat4 lightSpace;
    vec3 lightPos;
};

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject  {
    mat4 projection;
    mat4 view;
    vec4 ambient;
    vec3 cameraPosition;
    DirectionalLight directionalLight;
    bool haveDirectionalLight;
    int pointLightsCount;
    int shadowMapsCount;
} global;

layout(set = 0, binding = 1) uniform sampler2D texSampler[100];

layout(set = 0, binding = 2) uniform ModelUniformBufferObject  {
    mat4 matrix;
    mat4 normalMatrix;
} model;

layout(set = 0, binding = 3) uniform SurfaceUniformBufferObject  {
    int transparency;
    float alphaScissor;
    int diffuseIndex;
    int specularIndex;
    int normalIndex;
    vec4 albedoColor;
    float shininess;
} material;

layout(set = 0, binding = 4) uniform PointLightArray {
    PointLight lights[1];
} pointLights;

layout(set = 0, binding = 5) uniform ShadowMapArray {
    ShadowMap shadowMaps[1];
} shadowMapsInfos;

layout (set = 0, binding = 6) uniform sampler2D shadowMaps[1];