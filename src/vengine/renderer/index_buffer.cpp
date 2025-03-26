#include "index_buffer.hpp"

namespace Vengine {

IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t count) : m_count(count) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer() {
    glDeleteBuffers(1, &m_id);
}

auto IndexBuffer::bind() const -> void {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

auto IndexBuffer::unbind() const -> void {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

[[nodiscard]] auto IndexBuffer::getCount() const -> uint32_t {
    return m_count;
}

}  // namespace Vengine