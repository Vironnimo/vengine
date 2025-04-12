// first shader, passes data to the fragment shader

#version 330 core

layout(location = 0) in vec3 aPos; 
layout(location = 1) in vec2 aTexCoord; 
    
uniform mat4 uTransform;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec4 uColor;

out vec4 vertexColor;
out vec2 texCoord;

void main() {
    gl_Position = uProjection * uView * uTransform * vec4(aPos, 1.0);
    vertexColor = uColor;
    texCoord = aTexCoord;
}