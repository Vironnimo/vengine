#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <freetype/freetype.h>

#include "vengine/renderer/shader.hpp"
#include "vengine/renderer/vertex_array.hpp"
#include "vengine/renderer/vertex_buffer.hpp"

namespace Vengine {

class Font {
   public:
    struct Character {
        GLuint textureID;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };

    Font() = default;
    ~Font();

    [[nodiscard]] auto load(const std::string& fileName, FT_Library& ftLibrary) -> bool;
    auto unload() -> bool;

    auto draw(std::string text, float x, float y, float scale, const glm::vec4& color) -> void;

    // set these before calling load()
    void setFontSize(unsigned int size);
    void setShader(std::shared_ptr<Shader> shader);
    void setVao(std::shared_ptr<VertexArray> vao);
    void setVbo(std::shared_ptr<VertexBuffer> vbo);

    [[nodiscard]] auto getCharacter(char c) const -> const Character*;

   private:
    std::unordered_map<char, Character> m_characters;
    unsigned int m_fontSize = 24;
    bool m_isLoaded = false;

    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexArray> m_vao;
    std::shared_ptr<VertexBuffer> m_vbo;
};

}  // namespace Vengine