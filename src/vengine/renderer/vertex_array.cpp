#include "vertex_array.hpp"

#include <spdlog/spdlog.h>

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

auto VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, const VertexLayout& layout) const -> void {
    bind();
    vertexBuffer->bind();

    auto stride = layout.calculateStride();

    uintptr_t currentOffset = 0;  

    // Position attribute (location 0) 
    if (layout.hasPosition) {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void*)currentOffset);
        glEnableVertexAttribArray(0);
        currentOffset += 3 * sizeof(float);
    }

    // Texture coordinate attribute (location 1)
    if (layout.hasTexCoords) {
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)currentOffset);
        glEnableVertexAttribArray(1);
        currentOffset += 2 * sizeof(float);
    }

    // Normal attribute (location 2)
    if (layout.hasNormals) {
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (const void*)currentOffset);
        glEnableVertexAttribArray(2);
        currentOffset += 3 * sizeof(float);
    }

    vertexBuffer->unbind();
    unbind();  
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