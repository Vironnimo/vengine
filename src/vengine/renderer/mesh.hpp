#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "vertex_array.hpp"
#include "vertex_buffer.hpp"
#include "index_buffer.hpp" 

namespace Vengine {

class Mesh {
   public:
    Mesh(const std::vector<float>& vertices);
    Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh();

    auto draw() const -> void;

    [[nodiscard]] auto getBounds() const -> std::pair<glm::vec3, glm::vec3>; 
    [[nodiscard]] auto getVertexArray() const -> const std::shared_ptr<VertexArray>& {
        return m_vertexArray;
    }
    [[nodiscard]] auto getVertexBuffer() -> std::shared_ptr<VertexBuffer>& {
        return m_vertexBuffer;
    }
    [[nodiscard]] auto getVertexBuffer() const -> const std::shared_ptr<VertexBuffer>& {
        return m_vertexBuffer;
    }
    [[nodiscard]] auto getIndexBuffer() const -> const std::shared_ptr<IndexBuffer>& {
        return m_indexBuffer;
    }
    [[nodiscard]] auto useIndices() const -> bool {
        return m_useIndices;
    }
    [[nodiscard]] auto getVertexCount() const -> size_t;  

   private:
    std::vector<float> m_vertices; // keep raw data for who knows what?   
    std::vector<uint32_t> m_indices;  
    std::shared_ptr<VertexArray> m_vertexArray;
    std::shared_ptr<VertexBuffer> m_vertexBuffer;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
    bool m_useIndices = false;
};

}  // namespace Vengine