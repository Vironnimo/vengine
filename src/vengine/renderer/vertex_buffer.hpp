#pragma once
#include <glad/glad.h>
#include <cstdint>

namespace Vengine {

class VertexBuffer {
   public:
    VertexBuffer(const float* vertices, uint32_t size);
    ~VertexBuffer();

    auto bind() const -> void;
    auto unbind() const -> void;

    [[nodiscard]] auto getSize() const -> uint32_t;
    [[nodiscard]] auto getId() const -> GLuint;

   private:
    uint32_t m_size = 0;
    GLuint m_id = 0;
};

}  // namespace Vengine