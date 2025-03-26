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

   private:
    GLuint m_id = 0;
};

}  // namespace Vengine