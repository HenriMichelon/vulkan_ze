#version 450

#include "input_datas.glsl"

layout (location = 0) in vec2 UV;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec4 GLOBAL_POSITION;
layout (location = 3) in vec3 POSITION;

layout (location = 0) out vec4 COLOR;

vec3 normal;
vec3 viewDir;

vec3 calcDirectionalLight(DirectionalLight light, vec3 color) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w * color;
    if (material.specularIndex != -1) {
        // Blinn-Phong
        // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
        vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], UV).rgb;
        return diffuse + specular;
    }
    return diffuse;
}

vec3 calcPointLight(PointLight light, vec3 color) {
    vec3 directionToLight = light.position.xyz - GLOBAL_POSITION.xyz;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // attenuate by object distance squared
    directionToLight = normalize(directionToLight);
    vec3 intensity = light.color.xyz * light.color.w;
    bool cutOff = light.isSpot;
    if (light.isSpot) {
        float theta = dot(directionToLight, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        intensity *= clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        cutOff = theta <= light.outerCutOff;
    }

    if (!cutOff) {
        float cosAngIncidence = max(dot(normal, directionToLight), 0);
        vec3 diffuseLight = intensity * attenuation * cosAngIncidence * color;
         if (material.specularIndex != -1) {
            // Blinn-Phong
            // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
            vec3 halfwayDir = normalize(directionToLight + viewDir);
            float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
            vec3 specular = intensity * attenuation * light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], UV).rgb;
            return diffuseLight + specular;
        }
        return diffuseLight;
    }
    return vec3(0.0, 0, 0);
}

// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadowFactor(int shadowMapIndex)
{
    vec4 SHADOW_COORD = shadowMapsInfos.shadowMaps[shadowMapIndex].lightSpace * GLOBAL_POSITION;
    vec3 LIGHT_DIR = normalize(shadowMapsInfos.shadowMaps[shadowMapIndex].lightPos - POSITION);

    vec3 projCoords = SHADOW_COORD.xyz / SHADOW_COORD.w;
    if (projCoords.z > 1.0) return 1.0f;
    // Remap xy to [0.0, 1.0]
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.001f || projCoords.x > 0.999f || projCoords.y < 0.001f || projCoords.y > 0.999f);
    if (outOfView) return 1.0f;

    float currentDepth = projCoords.z;
    float closestDepth = texture(shadowMaps[shadowMapIndex], projCoords.xy).r;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[shadowMapIndex], 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMaps[shadowMapIndex], projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return 1.0 - shadow;
}


void main() {
    viewDir = normalize(global.cameraPosition - GLOBAL_POSITION.xyz);
    vec4 color = material.albedoColor;
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], UV);
    }

    if (material.normalIndex != -1) {
        normal = texture(texSampler[material.normalIndex], UV).rgb;
        normal = normalize(normal * 2.0 - 1.0) ;
        normal.y = -normal.y;
        normal.z = -normal.z;
    } else {
        normal = NORMAL;
    }

    if (((material.transparency == 2) || (material.transparency == 3)) && (color.a < material.alphaScissor)) {
        discard;
    }

    vec3 ambient = global.ambient.w * global.ambient.rgb * color.rgb;

    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        diffuse = calcDirectionalLight(global.directionalLight, color.rgb);
    }
    for(int i = 0; i < global.pointLightsCount; i++) {
        diffuse += calcPointLight(pointLights.lights[i], color.rgb);
    }
    vec3 result = ambient + diffuse;

    for (int i = 0; i < global.shadowMapsCount; i++) {
        float shadows = shadowFactor(i);
        result = (ambient + shadows) * result;
    }

    COLOR = vec4(result, material.transparency == 1 || material.transparency == 3 ? color.a : 1.0);

}