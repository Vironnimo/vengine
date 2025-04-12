// gets data from vertex shader

#version 330 core

in vec2 texCoord;
in vec4 vertexColor;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;

void main() {
    if (uUseTexture) {
        FragColor = texture(uTexture, texCoord);
    } else {
        FragColor = vertexColor;
    }
}