#pragma once
#include <glad/glad.h>
#include <cstdint>

namespace Vengine {

class VertexBuffer {
   public:
    VertexBuffer(const float* vertices, uint32_t size, bool hasTexCoords = false);
    ~VertexBuffer();

    auto bind() const -> void;
    auto unbind() const -> void;

    [[nodiscard]] auto hasTexCoords() const -> bool;

   private:
    GLuint m_id = 0;
    bool m_hasTexCoords = false;  
};

}  // namespace Vengine