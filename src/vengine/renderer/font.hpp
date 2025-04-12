#pragma once

#include <string>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "vengine/renderer/shader.hpp"

namespace Vengine {

class Font {
   public:
    struct Character {
        GLuint textureID;      // ID handle of the glyph texture
        glm::ivec2 size;       // Size of glyph
        glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
        unsigned int advance;  // Offset to advance to next glyph
    };

    Font() = default;
    ~Font() {
        unload();
    }

    auto load(const std::string& fileName) -> bool;
    auto unload() -> bool;

    // Set the font size to use when loading
    void setFontSize(unsigned int size);
    // Use this function before rendering text
    void use();
    void renderText(const std::string& text, float x, float y, float scale = 1.0f,
                    const glm::vec4& color = glm::vec4(1.0f));

   private:
    std::unordered_map<char, Character> m_characters;
    unsigned int m_fontSize = 24;
    bool m_isLoaded = false;

    // Static members for shared text rendering resources
    static bool s_initialized;
    static GLuint s_vao;
    static GLuint s_vbo;
    static std::shared_ptr<Shader> s_textShader;

    // Initialize text rendering resources (VAO, VBO, shader)
    static void initTextRendering() {
        if (s_initialized) {
            return;
}

        // Create shader
        s_textShader = std::make_shared<Shader>("text", "resources/shaders/text.vert", "resources/shaders/text.frag");

        // Configure VAO/VBO for texture quads
        glGenVertexArrays(1, &s_vao);
        glGenBuffers(1, &s_vbo);
        glBindVertexArray(s_vao);
        glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        s_initialized = true;
    }

    // Cleanup text rendering resources
    static void cleanupTextRendering() {
        if (!s_initialized) {
            return;
        }

        glDeleteVertexArrays(1, &s_vao);
        glDeleteBuffers(1, &s_vbo);
        s_textShader = nullptr;
        s_initialized = false;
    }
};

}  // namespace Vengine