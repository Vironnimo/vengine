#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <vector>


namespace Vengine {

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;
};

struct Health {
    int health;
};

struct RenderData {
    GLuint shaderProgramId = 0;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    GLuint vao = 0;
    GLuint vbo = 0;
};

struct Mesh {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> texCoords;

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        this->vertices = vertices;
        this->indices = indices;
        setupMesh();
    }

    void setupMesh() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    ~Mesh() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }
};

}  // namespace Engine