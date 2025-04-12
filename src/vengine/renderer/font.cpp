#include "font.hpp"

#include <filesystem>

namespace Vengine {

bool Font::s_initialized = false;
GLuint Font::s_vao = 0;
GLuint Font::s_vbo = 0;
std::shared_ptr<Shader> Font::s_textShader = nullptr;

auto Font::load(const std::string& fileName) -> bool {
    if (m_isLoaded) {
        unload();
    }

    // Initialize FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft) != 0) {
        spdlog::error("Failed to initialize FreeType library");
        return false;
    }

    // Find font file
    auto folder = std::filesystem::path("resources/fonts");
    auto fullPath = folder / fileName;

    if (!std::filesystem::exists(fullPath)) {
        spdlog::error("Font file not found: {}", fullPath.string());
        FT_Done_FreeType(ft);
        return false;
    }

    // Load font face
    FT_Face face;
    if (FT_New_Face(ft, fullPath.string().c_str(), 0, &face) != 0) {
        spdlog::error("Failed to load font: {}", fullPath.string());
        FT_Done_FreeType(ft);
        return false;
    }

    // Set font size
    FT_Set_Pixel_Sizes(face, 0, m_fontSize);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 ASCII characters
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) {
            spdlog::warn("Failed to load glyph: {}", c);
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store character
        Character character = {texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               static_cast<unsigned int>(face->glyph->advance.x)};
        m_characters.insert(std::pair<char, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // Clean up FreeType resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    if (!s_initialized) {
        initTextRendering();
    }

    m_isLoaded = true;
    return true;
}

auto Font::unload() -> bool {
    if (!m_isLoaded) {
        return false;
    }

    // Delete character textures
    for (auto& [_, character] : m_characters) {
        glDeleteTextures(1, &character.textureID);
    }

    m_characters.clear();
    m_isLoaded = false;
    return true;
}

// Set the font size to use when loading
void Font::setFontSize(unsigned int size) {
    m_fontSize = size;
}

// Use this function before rendering text
void Font::use() {
    if (!s_initialized) {
        spdlog::error("Text rendering not initialized. Load a font first");
        return;
    }

    if (!m_isLoaded) {
        spdlog::error("Cannot use font - not loaded");
        return;
    }

    // Bind shader and setup projection
    s_textShader->bind();

    // Create orthographic projection for text
    GLFWwindow* window = glfwGetCurrentContext();
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    s_textShader->setUniformMat4("projection", projection);
}

void Font::renderText(const std::string& text, float x, float y, float scale, const glm::vec4& color) {
    if (!s_initialized || !m_isLoaded) {
        return;
    }

    // Save previous state
    GLint lastProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
    GLboolean depthEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);

    // Disable depth testing for text
    glDisable(GL_DEPTH_TEST);

    // Activate shader and set uniforms
    s_textShader->bind();
    s_textShader->setUniformVec4("textColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(s_vao);

    // Iterate through characters
    float startX = x;
    for (char c : text) {
        if (c == '\n') {
            y -= 30.0f * scale;  // Line height
            x = startX;
            continue;
        }

        // Skip if character not found
        if (m_characters.find(c) == m_characters.end()) {
            x += 10.0f * scale;  // Default advance
            continue;
        }

        const Character& ch = m_characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // Update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},     {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f}, {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance cursor for next glyph
        x += (ch.advance >> 6) * scale;  // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    // Restore state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(lastProgram);

    if (depthEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

}  // namespace Vengine