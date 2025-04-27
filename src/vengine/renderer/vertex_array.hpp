#pragma once

#include <glad/glad.h>
#include <memory>
#include "index_buffer.hpp"
#include "vertex_buffer.hpp"
#include "vertex_layout.hpp"

namespace Vengine {

class Mesh;

class VertexArray {
   public:
    VertexArray();
    ~VertexArray();

    auto bind() const -> void;
    auto unbind() const -> void;

    auto addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, const VertexLayout& layout) const -> void;
    auto addIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) -> void;

    [[nodiscard]] auto getIndexBuffer() const -> std::shared_ptr<IndexBuffer>;
   private:
    GLuint m_id = 0;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
};

}  // namespace Vengine