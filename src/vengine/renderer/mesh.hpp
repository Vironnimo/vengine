#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "vertex_array.hpp"
#include "vertex_buffer.hpp"
#include "index_buffer.hpp"  // Added missing include

namespace Vengine {

class Mesh {
   public:
    Mesh(const std::vector<float>& vertices);
    Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh();

    // Removed transform methods: setPosition, setRotation, setScale, getTransform

    auto draw() const -> void;

    // Added getter for vertex buffer info needed in draw calculation
    [[nodiscard]] auto getVertexBuffer() const -> const std::shared_ptr<VertexBuffer>& {
        return m_vertexBuffer;
    }
    [[nodiscard]] auto getIndexBuffer() const -> const std::shared_ptr<IndexBuffer>& {
        return m_indexBuffer;
    }
    [[nodiscard]] auto useIndices() const -> bool {
        return m_useIndices;
    }
    [[nodiscard]] auto getVertexCount() const -> size_t;  // Helper to get vertex count

   private:
    std::vector<float> m_vertices;    // Keep raw data for potential re-use? Optional.
    std::vector<uint32_t> m_indices;  // Keep raw data for potential re-use? Optional.
    std::shared_ptr<VertexArray> m_vertexArray;
    std::shared_ptr<VertexBuffer> m_vertexBuffer;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
    bool m_useIndices = false;
};

}  // namespace Vengine