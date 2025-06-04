#version 330 core

in vec2 texCoord;
in vec4 vertexColor;
in vec3 fs_Normal;
in vec3 fs_FragPos;
in vec4 fs_FragPosLightSpace;  // Interpolated from vertex shader

out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;

uniform vec3 uLightDirection = vec3(-0.5, -0.7, -0.5);  // Light pointing down-back-left
uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);
uniform float uAmbientStrength = 0.3;  // shadow strength

uniform vec3 uDiffuse = vec3(1.0, 1.0, 1.0);  
uniform vec3 uAmbient = vec3(1.0, 1.0, 1.0);  
uniform vec3 uSpecular = vec3(1.0, 1.0, 1.0); 
uniform float uShininess = 32.0; 

uniform sampler2D uShadowMap;
uniform mat4 uLightSpaceMatrix;

uniform vec3 uViewPos;

void main() {
    // Determine base color (from texture or vertex color)
    vec4 baseColor;
    if (uUseTexture) {
        baseColor = texture(uTexture, texCoord);
        if (baseColor.a < 0.1)
            discard;  // Discard transparent pixels
    } else {
        baseColor = vertexColor;
    }

    // Apply material diffuse color to the base color
    baseColor.rgb *= uDiffuse;
    
    // Prepare normals and light direction
    vec3 norm = normalize(fs_Normal);
    vec3 lightDir = normalize(-uLightDirection);

    // Ambient component
    vec3 ambient = uAmbientStrength * uLightColor * uAmbient;

    // Diffuse component
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * uDiffuse;

    // Specular component (view-dependent)
    vec3 viewDir = normalize(uViewPos - fs_FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 specular = uSpecular * spec * uLightColor;

    // Shadow calculation (unchanged)
    // vec4 fragPosLightSpace = uLightSpaceMatrix * vec4(fs_FragPos, 1.0);
    vec4 fragPosLightSpace = fs_FragPosLightSpace;  // Use the interpolated value from vertex shader
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;  // to [0,1] range

    float shadow = 1.0;
    // float bias = 0.005;
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    shadow = 1.0 - shadow;  // invert so 1.0 = lit, 0.0 = shadowed
    
    // Prevent shadow outside the light's frustum
    if(projCoords.z > 1.0)
        shadow = 1.0;

    // Combine lighting components
    // Ambient is not affected by shadows, diffuse and specular are
    vec3 result = (ambient + shadow * (diffuse + specular)) * baseColor.rgb;

    // Final color
    FragColor = vec4(result, baseColor.a);
}