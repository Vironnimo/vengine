#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "vengine/renderer/shader.hpp"
#include "vengine/renderer/vertex_array.hpp"
#include "vengine/renderer/vertex_buffer.hpp"
#include "vengine/core/resources.hpp"

namespace Vengine {

class Skybox {
   public:
    Skybox();
    ~Skybox();

    [[nodiscard]] auto load(const std::vector<std::string>& faceFiles) -> bool;
    [[nodiscard]] auto loadFromTextures(const std::vector<std::shared_ptr<Texture>>& textures) -> bool;

    auto render(const glm::mat4& view, const glm::mat4& projection) -> void;

    auto setShader(std::shared_ptr<Shader> shader) -> void;

   private:
    GLuint m_textureID = 0;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexArray> m_vao;
    std::shared_ptr<VertexBuffer> m_vbo;
};

}  // namespace Vengine