#version 330 core

in vec2 texCoord;
in vec4 vertexColor;
in vec3 fs_Normal;
in vec3 fs_FragPos;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;
uniform vec3 uLightDirection = vec3(0.5, 11.0, 10.3);
uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);
uniform float uAmbientStrength = 0.3;  // light strenght
uniform sampler2D uShadowMap;
uniform mat4 uLightSpaceMatrix;

void main() {
    vec4 baseColor;
    if (uUseTexture) {
        baseColor = texture(uTexture, texCoord);
    } else {
        baseColor = vertexColor;
    }

    vec3 ambient = uAmbientStrength * uLightColor;

    vec3 norm = normalize(fs_Normal);
    vec3 lightDir = -normalize(uLightDirection);

    // Calculate diffuse factor (how much the surface faces the light)
    float diff = max(dot(norm, lightDir), 0.0);  // Use max to prevent negative light
    vec3 diffuse = diff * uLightColor;

    vec4 fragPosLightSpace = uLightSpaceMatrix * vec4(fs_FragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;  // to [0,1] range

    float closestDepth = texture(uShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // float shadow = currentDepth - 0.005 > closestDepth ? 0.5 : 1.0;  // 0.5 = in shadow
    // PCF soft shadow
    float shadow = 0.0;
    float bias = 0.005;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    shadow = 1.0 - shadow;  // invert so 1.0 = lit, 0.0 = shadowed

    vec3 result = (ambient + shadow * diffuse) * baseColor.rgb;

    // final color
    FragColor = vec4(result, baseColor.a);  
}