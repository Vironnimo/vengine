#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 4) in mat4 uTransform; // For instancing, if you use it

uniform mat4 uLightSpaceMatrix;

void main() {
    gl_Position = uLightSpaceMatrix * uTransform * vec4(aPos, 1.0);
}