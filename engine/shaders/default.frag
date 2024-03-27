#version 450

#include "input_datas.glsl"
layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec3 normal;
vec4 color;

#include "lighting.glsl"

// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadowFactor(int shadowMapIndex) {
    vec4 SHADOW_COORD = shadowMapsInfos.shadowMaps[shadowMapIndex].lightSpace * fs_in.GLOBAL_POSITION;
    vec3 LIGHT_DIR = normalize(shadowMapsInfos.shadowMaps[shadowMapIndex].lightPos - fs_in.POSITION);

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
    for(int x = -1; x <= 1; ++x)  {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMaps[shadowMapIndex], projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return 1.0 - shadow;
}


void main() {
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], fs_in.UV);
    } else {
        color = material.albedoColor;
    }

    if (((material.transparency == 2) || (material.transparency == 3)) && (color.a < material.alphaScissor)) {
        discard;
    }

    if ((material.normalIndex != -1) && (fs_in.tangent != vec4(0.0, 0.0, 0.0, 0.0))) {
        normal = texture(texSampler[material.normalIndex], fs_in.UV).rgb * 2.0 - 1.0;
        normal = normalize(fs_in.TBN * normal);
    } else {
        normal = fs_in.NORMAL;
    }
    //COLOR = vec4(normal, 1.0);

    vec3 ambient = global.ambient.w * global.ambient.rgb * color.rgb;
    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        diffuse = calcDirectionalLight(global.directionalLight);
    }
    for(int i = 0; i < global.pointLightsCount; i++) {
        diffuse += calcPointLight(pointLights.lights[i]);
    }
    vec3 result = ambient + diffuse;

    for (int i = 0; i < global.shadowMapsCount; i++) {
        float shadows = shadowFactor(i);
        result = (ambient + shadows) * result;
    }

    COLOR = vec4(result, material.transparency == 1 || material.transparency == 3 ? color.a : 1.0);
}