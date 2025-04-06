#include "vertex_buffer.hpp"

#include <spdlog/spdlog.h>

namespace Vengine {

VertexBuffer::VertexBuffer(const float* vertices, uint32_t size, bool hasTexCoords) : m_hasTexCoords(hasTexCoords) {
    spdlog::debug("Constructor VertexBuffer, vertices size: {}, hasTexture: {}", size, hasTexCoords);
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
    spdlog::debug("Destructor VertexBuffer");
    glDeleteBuffers(1, &m_id);
}

auto VertexBuffer::bind() const -> void {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

auto VertexBuffer::unbind() const -> void {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto VertexBuffer::hasTexCoords() const -> bool {
    return m_hasTexCoords;
}

}  // namespace Vengine