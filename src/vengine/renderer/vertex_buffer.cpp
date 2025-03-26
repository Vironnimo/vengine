#include "vertex_buffer.hpp"

namespace Vengine {

VertexBuffer::VertexBuffer(const float* vertices, uint32_t size) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &m_id);
}

auto VertexBuffer::bind() const -> void {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

auto VertexBuffer::unbind() const -> void {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}  // namespace Vengine