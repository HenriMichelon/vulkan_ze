vec3 calcDirectionalLight(DirectionalLight light) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w * color.rgb;
    if (material.specularIndex != -1) {
        // Blinn-Phong
        // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
        vec3 halfwayDir = normalize(lightDir + fs_in.VIEW_DIRECTION);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
        vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], fs_in.UV).rgb;
        return diffuse + specular;
    }
    return diffuse;
}

vec3 calcPointLight(PointLight light) {
    vec3 directionToLight = light.position.xyz - fs_in.GLOBAL_POSITION.xyz;
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
        vec3 diffuseLight = intensity * attenuation * cosAngIncidence * color.rgb;
        if (material.specularIndex != -1) {
            vec3 halfwayDir = normalize(directionToLight + fs_in.VIEW_DIRECTION);
            float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
            vec3 specular = intensity * attenuation * light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], fs_in.UV).rgb;
            return diffuseLight + specular;
        }
        return diffuseLight;
    }
    return vec3(0.0, 0, 0);
}