#include "vertex_array.hpp"

namespace Vengine {

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_id);
}

auto VertexArray::bind() const -> void {
    glBindVertexArray(m_id);
}

auto VertexArray::unbind() const -> void {
    glBindVertexArray(0);
}

auto VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) const -> void {
    bind();
    vertexBuffer->bind();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
}

auto VertexArray::addIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) -> void {
    bind();
    indexBuffer->bind();
    m_indexBuffer = std::move(indexBuffer);
}

[[nodiscard]] auto VertexArray::getIndexBuffer() const -> std::shared_ptr<IndexBuffer> {
    return m_indexBuffer;
}

}  // namespace Vengine