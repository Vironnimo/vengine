#include "vertex_array.hpp"

#include <spdlog/spdlog.h>

namespace Vengine {

VertexArray::VertexArray() {
    spdlog::debug("Constructor VertexArray");
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray() {
    spdlog::debug("Destructor VertexArray");
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

    if (vertexBuffer->hasTexCoords()) {
        const int stride = 5 * sizeof(float);
        // position attribute (location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(0);

        // texture coordinate attribute (location 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    } else {
        const int stride = 3 * sizeof(float);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(0);
    }
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