#include "vertex_buffer.hpp"

#include <spdlog/spdlog.h>

namespace Vengine {

VertexBuffer::VertexBuffer(const float* vertices, uint32_t size)
    : m_size(size) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, m_size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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

[[nodiscard]] auto VertexBuffer::getSize() const -> uint32_t {
    return m_size;
}

[[nodiscard]] auto VertexBuffer::getId() const -> GLuint {
    return m_id;
}

}  // namespace Vengine