#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
// #include "vertex_array.hpp"
#include "vengine/core/i_resource.hpp"
#include "vengine/renderer/vertex_buffer.hpp"
#include "vengine/renderer/index_buffer.hpp"
#include "vengine/renderer/vertex_layout.hpp"

namespace Vengine {

class VertexArray;

struct Submesh {
    uint32_t indexOffset;
    uint32_t indexCount;
    std::string materialName;  
};

class Mesh : public IResource {
   public:
    Mesh() = default;
    Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, VertexLayout layout);
    ~Mesh() override;

    auto load(const std::string& fileName) -> bool override;
    auto unload() -> bool override;
    auto finalizeOnMainThread() -> bool override;

    [[nodiscard]] auto getBounds() const -> std::pair<glm::vec3, glm::vec3>;
    [[nodiscard]] auto getVertexArray() const -> const std::shared_ptr<VertexArray>& {
        return m_vertexArray;
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
    [[nodiscard]] auto getFloatsPerVertex() const -> int;

    [[nodiscard]] auto getVertexLayout() const -> const VertexLayout& {
        return m_layout;
    }

    [[nodiscard]] auto getVerticesRaw() const -> const std::vector<float>& {
        return m_vertices;
    }
    [[nodiscard]] auto getIndicesRaw() const -> const std::vector<uint32_t>& {
        return m_indices;
    }

    [[nodiscard]] auto getSubmeshes() const -> const std::vector<Submesh>& {
        return m_submeshes;
    }

    auto addSubmesh(const Submesh& submesh) -> void {
        m_submeshes.push_back(submesh);
    }

   private:
    std::vector<float> m_vertices;  // keep raw data for who knows what?
    std::vector<uint32_t> m_indices;
    std::shared_ptr<VertexArray> m_vertexArray;
    std::shared_ptr<VertexBuffer> m_vertexBuffer;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
    bool m_useIndices = false;
    VertexLayout m_layout;

    std::vector<Submesh> m_submeshes;
};

}  // namespace Vengine