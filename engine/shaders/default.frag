#version 450

#include "input_datas.glsl"

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec3 POSITION;

layout(location = 0) out vec4 COLOR;

vec3 calcDirectionalLight(DirectionalLight light, vec3 color) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(NORMAL, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w * color;
    if (material.specularIndex != -1) {
        vec3 viewDir = normalize(global.cameraPosition - POSITION);
        vec3 reflectDir = reflect(- lightDir, NORMAL);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], UV).rgb;
        return diffuse + specular;
    }
    return diffuse;
}

vec3 calcPointLight(PointLight light, vec3 color) {
    float distance    = length(global.light.position - POSITION);
    float attenuation = 1.0 / (1.0 + global.light.linear * distance + global.light.quadratic * (distance * distance));
    vec3 lightDir = normalize(global.light.position - POSITION);
    float diff = max(dot(NORMAL, lightDir), 0.0);
    vec3 diffuse = attenuation * diff * global.light.color.rgb * global.light.color.w * color;
    if (material.specularIndex != -1) {
        vec3 viewDir = normalize(global.cameraPosition - POSITION);
        vec3 reflectDir = reflect(-lightDir, NORMAL);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = global.light.specular * spec * global.light.color.rgb * texture(texSampler[material.specularIndex], UV).rgb;
        return attenuation * diffuse + specular;
    }
    return diffuse;
}

vec3 calcSpotLight(SpotLight light, vec3 color) {
    float distance    = length(global.light.position - POSITION);
    float attenuation = 1.0 / (1.0 + global.light.linear * distance + global.light.quadratic * (distance * distance));
    vec3 lightDir = normalize(global.light.position - POSITION);

    float theta = dot(lightDir, -global.light.direction);
    float epsilon   = global.light.cutOff - global.light.outerCutOff;
    float intensity = clamp((theta - global.light.outerCutOff) / epsilon, 0.0, 1.0);

    if (theta > global.light.outerCutOff)
    {
        // diffuse
        float diff = max(dot(NORMAL, lightDir), 0.0);
        vec3 diffuse = intensity * attenuation * diff * global.light.color.rgb * global.light.color.w * color;

        // specular
        if (material.specularIndex != -1) {
            vec3 viewDir = normalize(global.cameraPosition - POSITION);
            vec3 reflectDir = reflect(-lightDir, NORMAL);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
            vec3 specular = intensity * attenuation * global.light.specular * spec * global.light.color.rgb * texture(texSampler[material.specularIndex], UV).rgb;
            return diffuse + specular;
        }
        return diffuse;
    }
    return vec3(0, 0, 0);
}

void main() {
    vec3 color = material.albedoColor.rgb;
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], UV).rgb;
    }
    // ambient
    vec3 ambient = global.ambient.w * global.ambient.rgb * color;

    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        diffuse = calcDirectionalLight(global.directionalLight, color);
    }

    //diffuse += calcPointLight(global.light, color);
    //diffuse += calcSpotLight(global.light, color);

    vec3 result = (ambient + diffuse) * material.albedoColor.rgb;
    COLOR = vec4(result, 1.0);

}