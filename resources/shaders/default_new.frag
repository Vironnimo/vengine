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
uniform float uAmbientStrength = 0.3; // light strenght

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

    vec3 result = (ambient + diffuse) * baseColor.rgb;

    // Final color
    // FragColor = vec4(normalize(fs_Normal) * 0.5 + 0.5, 1.0);
    FragColor = vec4(result, baseColor.a); // Keep original alpha
}

// void main() {
//     FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Solid red
// }