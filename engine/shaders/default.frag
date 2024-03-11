#version 450

#include "input_datas.glsl"

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec3 POSITION;

layout(location = 0) out vec4 COLOR;


/*vec3 calcDirectionalLight(DirectionalLight light, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(NORMAL, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, NORMAL);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, UV));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, UV));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, UV));
    return (ambient + diffuse + specular);
}*/

void main() {
    vec3 color = material.albedoColor.rgb;
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], UV).rgb;
    }

    // ambient
    vec3 ambient = global.directionalLight.ambient.w * global.directionalLight.ambient.rgb * color;

    // diffuse
    float diff = max(dot(NORMAL, global.directionalLight.direction), 0.0);
    vec3 diffuse = diff * global.directionalLight.color.rgb * global.directionalLight.color.w * color;

    // specular
    if (material.specularIndex != -1) {
        vec3 viewDir = normalize(global.cameraPosition - POSITION);
        vec3 reflectDir = reflect(-global.directionalLight.direction, NORMAL);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = global.directionalLight.specular.w * spec * global.directionalLight.color.rgb *
            //vec3(0.1,0.1,0.1);
            texture(texSampler[material.specularIndex], UV).rgb*2;
        diffuse = diffuse + specular;
    }

    vec3 result = (ambient + diffuse) * material.albedoColor.rgb;
    COLOR = vec4(result, 1.0);

}