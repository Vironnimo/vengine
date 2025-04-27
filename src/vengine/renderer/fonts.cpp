#include "fonts.hpp"

#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "vengine/renderer/vertex_layout.hpp"

namespace Vengine {

Fonts::Fonts() {
    spdlog::debug("Constructor Fonts");
}

Fonts::~Fonts() {
    spdlog::debug("Destructor Fonts");
    m_fonts.clear();
}

auto Fonts::init(std::shared_ptr<Shader> textShader) -> tl::expected<void, Error> {
    if (FT_Init_FreeType(&m_ft) != 0) {
        spdlog::error("Failed to initialize FreeType library");
        return tl::unexpected(Error{"Failed to initialize FreeType library"});
    }

    m_shader = std::move(textShader);
    m_vao = std::make_shared<VertexArray>();
    float vertices[] = {0.0f}; // empty for now, will be updated for each character
    m_vbo = std::make_shared<VertexBuffer>(vertices, sizeof(float) * 24);  // 6 vertices * 4 floats each

    // text quads (will be updated for each character)
    m_vao->bind();
    m_vbo->bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    // position attribute (x, y) and texture coordinates (s, t)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    m_vao->unbind();
    m_vbo->unbind();

    m_initialized = true;
    return {};
}

auto Fonts::load(const std::string& id, const std::string& filename, unsigned int fontSize) -> tl::expected<std::shared_ptr<Font>, Error> {
    if (m_fonts.find(id) != m_fonts.end()) {
        spdlog::warn("Font with ID '{}' already exists, overwriting", id);
    }

    auto font = std::make_shared<Font>();
    font->setFontSize(fontSize);
    font->setShader(m_shader);
    font->setVao(m_vao);
    font->setVbo(m_vbo);

    if (!font->load(filename, m_ft)) {
        spdlog::error("Failed to load font '{}' from file '{}'", id, filename);
        return tl::unexpected(Error{"Failed to load font"});
    }

    m_fonts[id] = font;
    return font;
}

auto Fonts::get(const std::string& id) -> tl::expected<std::shared_ptr<Font>, Error> {
    auto it = m_fonts.find(id);
    if (it == m_fonts.end()) {
        spdlog::error("Font '{}' not found", id);
        return tl::unexpected(Error{"Font not found"});
    }
    return it->second;
}

auto Fonts::has(const std::string& id) const -> bool {
    return m_fonts.find(id) != m_fonts.end();
}

auto Fonts::unload(const std::string& id) -> void {
    auto it = m_fonts.find(id);
    if (it != m_fonts.end()) {
        it->second->unload();
        m_fonts.erase(it);
    } else {
        spdlog::error("Font '{}' not found", id);
    }
}

auto Fonts::unloadAll() -> void {
    for (auto& [id, font] : m_fonts) {
        font->unload();
    }
    m_fonts.clear();
}

}  // namespace Vengine