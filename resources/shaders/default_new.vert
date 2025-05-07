#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 4) in mat4 uTransform; // Instanced attribute

// uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec4 uColor;

out vec4 vertexColor;
out vec2 texCoord;
out vec3 fs_Normal; 
out vec3 fs_FragPos; 

void main() {
    vec4 worldPos = uTransform * vec4(aPos, 1.0);
    fs_FragPos = worldPos.xyz;

    gl_Position = uProjection * uView * worldPos;

    vertexColor = uColor;
    texCoord = aTexCoord;

    // Transform the normal to world space (??)
    fs_Normal = mat3(transpose(inverse(uTransform))) * aNormal;
}