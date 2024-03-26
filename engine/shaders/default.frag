#version 450

#include "input_datas.glsl"

layout (location = 0) in vec2 UV;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec4 GLOBAL_POSITION;
layout (location = 3) in vec3 POSITION;
layout (location = 4) in vec3 VIEW_DIRECTION;

layout (location = 0) out vec4 COLOR;

vec3 normal;

#include "lighting.glsl"

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