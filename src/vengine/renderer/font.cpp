#include "font.hpp"

#include <filesystem>
#include <glm/ext/matrix_clip_space.hpp>
#include <utility>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <freetype/freetype.h>

namespace Vengine {

Font::~Font() {
    unload();
}

auto Font::draw(std::string text, float x, float y, float scale, const glm::vec4& color) -> void {
    if (!m_isLoaded) {
        spdlog::error("Cannot draw text - font not loaded");
        return;
    }
    if (!m_vao || !m_vbo || !m_shader) {
        spdlog::error("Cannot draw text - rendering resources not provided");
        return;
    }

    // previous state
    GLint lastProgram;
    GLboolean depthEnabled;
    glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
    glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);

    // no depth test for text, so it stays on top
    glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setUniformVec4("textColor", color);

    // create a 2d space for text with the size of the window (rotating the camera doesn't change the text position)
    GLFWwindow* window = glfwGetCurrentContext();
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    m_shader->setUniformMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    m_vao->bind();

    // draw each character
    float startX = x;
    for (char c : text) {
        if (c == '\n') {
            y -= m_fontSize * scale * 1.25f;  // line height i guess
            x = startX;
            continue;
        }

        auto it = m_characters.find(c);
        if (it == m_characters.end()) {
            x += 10.0f * scale;
            continue;
        }

        const Character& ch = it->second;

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);

        // update vertices for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},     {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f}, {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f}};
        m_vbo->bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance cursor for next glyph
        x += (ch.advance >> 6) * scale;  // bitshift by 6 to get value in pixels (2^6 = 64)
    }

    // restore state
    m_vao->unbind();
    m_vbo->unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(lastProgram);
    if (depthEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

auto Font::load(const std::string& fileName, FT_Library& ftLibrary) -> bool {
    if (m_isLoaded) {
        unload();
    }
    if (!ftLibrary) {
        spdlog::error("FreeType library not initialized");
        return false;
    }

    auto folder = std::filesystem::path("resources/fonts");
    auto fullPath = folder / fileName;
    if (!std::filesystem::exists(fullPath)) {
        spdlog::error("Font file not found: {}", fullPath.string());
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ftLibrary, fullPath.string().c_str(), 0, &face) != 0) {
        spdlog::error("Failed to load font: {}", fullPath.string());
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, m_fontSize);

    // disable byte-alignment restriction (?)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load first 128 ASCII characters (should this be increased?)
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != 0) {
            spdlog::warn("Failed to load glyph: {}", c);
            continue;
        }

        // generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               static_cast<unsigned int>(face->glyph->advance.x)};
        m_characters.insert(std::pair<char, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    m_isLoaded = true;

    return true;
}

auto Font::unload() -> bool {
    if (!m_isLoaded) {
        return false;
    }

    for (auto& [_, character] : m_characters) {
        glDeleteTextures(1, &character.textureID);
    }

    m_characters.clear();
    m_isLoaded = false;
    return true;
}

auto Font::getCharacter(char c) const -> const Font::Character* {
    auto it = m_characters.find(c);
    if (it == m_characters.end()) {
        return nullptr;
    }
    return &it->second;
}

void Font::setFontSize(unsigned int size) {
    m_fontSize = size;
}

void Font::setShader(std::shared_ptr<Shader> shader) {
    m_shader = std::move(shader);
}

void Font::setVao(std::shared_ptr<VertexArray> vao) {
    m_vao = std::move(vao);
}

void Font::setVbo(std::shared_ptr<VertexBuffer> vbo) {
    m_vbo = std::move(vbo);
}

}  // namespace Vengine