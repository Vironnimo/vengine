// gets data from vertex shader

// #version 330 core

// in vec4 vertexColor;
// out vec4 fragColor;

// void main() {
//     fragColor = vertexColor;
// }

#version 330 core
in vec4 vertexColor;
out vec4 FragColor;
void main() {
    FragColor = vertexColor;
}