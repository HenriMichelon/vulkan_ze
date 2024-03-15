#version 450

#include "input_datas.glsl"

layout (location = 0) in vec2 UV;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec3 POSITION;
layout (location = 4) in vec4 SHADOW_COORD;
/*layout (location = 5) in vec3 LIGHT;*/

layout (location = 0) out vec4 COLOR;

layout (binding = 5) uniform sampler2D shadowMap;

vec3 viewDir;
/*const int enablePCF = 0;

float textureProj(vec4 shadowCoord, vec2 off)
{
    float shadow = 1.0;
    if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 )
    {
        float dist = texture( shadowMap, shadowCoord.st + off ).r;
        if ( shadowCoord.w > 0.0 && dist < shadowCoord.z )
        {
            shadow = global.ambient.w;
        }
    }
    return shadow;
}

float filterPCF(vec4 sc)
{
    ivec2 texDim = textureSize(shadowMap, 0);
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
            count++;
        }
    }
    return shadowFactor / count;
}
*/
vec3 calcDirectionalLight(DirectionalLight light, vec3 color) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(NORMAL, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w * color;
    if (material.specularIndex != -1) {
        // Blinn-Phong
        // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(NORMAL, halfwayDir), 0.0), material.shininess*3);
        // Phong
        //vec3 reflectDir = reflect(- lightDir, NORMAL);
        //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], UV).rgb;
        return diffuse + specular;
    }
    return diffuse;
}

vec3 calcPointLight(PointLight light, vec3 color) {
    float dist = length(light.position - POSITION);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    vec3 lightDir = normalize(light.position - POSITION);
    float intensity = 1.0f;
    bool cutOff = light.isSpot;

    if (light.isSpot)
    {
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        cutOff = theta <= light.outerCutOff;
    }

    if (!cutOff)
    {
        float diff = max(dot(NORMAL, lightDir), 0.0);
        vec3 diffuse = intensity * attenuation * diff * light.color.rgb * light.color.w * color;
        if (material.specularIndex != -1) {
            // Blinn-Phong
            // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(NORMAL, halfwayDir), 0.0), material.shininess*3);
            // Phong
            //vec3 reflectDir = reflect(-lightDir, NORMAL);
            //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
            vec3 specular = intensity * attenuation * light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], UV).rgb;
            return diffuse + specular;
        }
        return diffuse;
    }
    return vec3(0, 0, 0);
}

// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float calcShadows()
{
    const vec3 posLightSpaceNDC  = SHADOW_COORD.xyz/SHADOW_COORD.w;    // for orto matrix, we don't need perspective division, you can remove it if you want; this is general case;
    const vec2 shadowTexCoord    = posLightSpaceNDC.xy*0.5f + vec2(0.5f, 0.5f);  // just shift coords from [-1,1] to [0,1]

    vec4 color = textureLod(shadowMap, shadowTexCoord, 0);
    bool shadow = (posLightSpaceNDC.z < color.x + 0.001f);

    const bool outOfView = (shadowTexCoord.x < 0.001f || shadowTexCoord.x > 0.999f || shadowTexCoord.y < 0.001f || shadowTexCoord.y > 0.999f);
    return (shadow || outOfView) ? 1.0f : 0.0f;
}

void main() {
    viewDir = normalize(global.cameraPosition - POSITION);
    vec3 color = material.albedoColor.rgb;
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], UV).rgb;
    }
    vec3 ambient = global.ambient.w * global.ambient.rgb * color;

    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        diffuse = calcDirectionalLight(global.directionalLight, color);
    }
    for(int i = 0; i < global.pointLightsCount; i++) {
        diffuse += calcPointLight(pointLights.lights[i], color);
    }
    vec3 result = (ambient + diffuse) * material.albedoColor.rgb;

    // shadows
    if (global.haveShadowMap)
    {
        float shadow = calcShadows();
        result = (ambient + shadow) * result;

        /*float shadow = (enablePCF == 1) ? filterPCF(SHADOW_COORD / SHADOW_COORD.w) : textureProj(SHADOW_COORD / SHADOW_COORD.w, vec2(0.0));
        vec3 N = normalize(NORMAL);
        vec3 L = normalize(LIGHT);
        result = max(dot(N, L),  global.ambient.w) * result * shadow;*/
    }
    COLOR = vec4(result, 1.0);

}