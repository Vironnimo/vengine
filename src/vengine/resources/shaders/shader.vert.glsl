#version 330 core

layout(location = 0) in vec3 position;
// layout(location = 1) in vec4 color;

out vec4 vertexColor;

uniform mat4 transform;
uniform vec4 uColor;

void main() {
    gl_Position = transform * vec4(position, 1.0f);
    // gl_Position = vec4(position.x, position.y, position.z, 1.0f);
    // vertexColor = color;
    vertexColor = uColor;
}