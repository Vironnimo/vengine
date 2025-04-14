#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <freetype/freetype.h>

#include "vengine/renderer/font.hpp"
#include "vengine/renderer/shader.hpp"
#include "vengine/core/error.hpp"
#include "vengine/renderer/vertex_array.hpp"
#include "vengine/renderer/vertex_buffer.hpp"

namespace Vengine {

class Fonts {
   public:
    Fonts();
    ~Fonts();

    [[nodiscard]] auto init(std::shared_ptr<Shader> textShader) -> tl::expected<void, Error>;
    [[nodiscard]] auto load(const std::string& id, const std::string& filename,
                            unsigned int fontSize = 24) -> tl::expected<std::shared_ptr<Font>, Error>;
    [[nodiscard]] auto get(const std::string& id) -> tl::expected<std::shared_ptr<Font>, Error>;
    [[nodiscard]] auto has(const std::string& id) const -> bool;
    auto unload(const std::string& id) -> void;

    auto unloadAll() -> void;

   private:
    bool m_initialized = false;
    FT_Library m_ft;
    std::unordered_map<std::string, std::shared_ptr<Font>> m_fonts;

    // NOTE for now just one shader, one vao and one vbo for each font
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexArray> m_vao;
    std::shared_ptr<VertexBuffer> m_vbo;
};

}  // namespace Vengine