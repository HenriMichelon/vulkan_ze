#version 450

#include "input_datas.glsl"

layout (location = 0) in vec2 UV;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec3 POSITION;
layout (location = 4) in vec4 SHADOW_COORD;
layout (location = 5) in vec3 LIGHT_DIR;

layout (location = 0) out vec4 COLOR;

layout (binding = 5) uniform sampler2D shadowMap;

vec3 viewDir;

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
float shadowFactor()
{
    vec3 projCoords = SHADOW_COORD.xyz / SHADOW_COORD.w;
    if (projCoords.z > 1.0) return 1.0f;
    // Remap xy to [0.0, 1.0]
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.001f || projCoords.x > 0.999f || projCoords.y < 0.001f || projCoords.y > 0.999f);
    if (outOfView) return 1.0f;

    float currentDepth = projCoords.z;
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return 1.0 - shadow;
}


void main() {
    viewDir = normalize(global.cameraPosition - POSITION);
    vec4 color = material.albedoColor;
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], UV);
    }
    //if (color.a < 0.1) discard;

    vec3 ambient = global.ambient.w * global.ambient.rgb * color.rgb;

    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        diffuse = calcDirectionalLight(global.directionalLight, color.rgb);
    }
    for(int i = 0; i < global.pointLightsCount; i++) {
        diffuse += calcPointLight(pointLights.lights[i], color.rgb);
    }
    vec3 result = (ambient + diffuse) * material.albedoColor.rgb;

    // shadows
    if (global.haveShadowMap)
    {
        float shadow = shadowFactor();
        result = (ambient + shadow) * result;
    }
    COLOR = vec4(result, color.a);

}