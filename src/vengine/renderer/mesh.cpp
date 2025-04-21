#include "mesh.hpp"

#include <glad/glad.h>
// #include <glm/gtc/matrix_transform.hpp> // No longer needed here
#include <spdlog/spdlog.h>

namespace Vengine {

// Constructor implementations remain largely the same, just ensure VertexBuffer layout is set correctly
Mesh::Mesh(const std::vector<float>& vertices) : m_vertices(vertices) {
    spdlog::debug("Constructor Mesh without indices, vertices count: {}", m_vertices.size());
    m_vertexArray = std::make_shared<VertexArray>();
    // Assuming 3 floats per vertex (pos) if no indices. Adjust if needed.
    // You might need a way to specify the layout (e.g., does it have tex coords?)
    // For now, assuming only position data if no indices.
    bool hasTexCoords = false;  // Determine this based on data or constructor args
    m_vertexBuffer = std::make_shared<VertexBuffer>(m_vertices.data(), m_vertices.size() * sizeof(float), hasTexCoords);

    m_vertexArray->addVertexBuffer(m_vertexBuffer);
}

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
    : m_vertices(vertices), m_indices(indices), m_useIndices(true) {
    spdlog::debug("Constructor Mesh with indices, indices count: {}, vertices count: {}", m_indices.size(),
                  m_vertices.size());
    m_vertexArray = std::make_shared<VertexArray>();
    // Assuming 5 floats per vertex (pos + tex) if indices are used (common case from loader). Adjust if needed.
    bool hasTexCoords = true;  // Determine this based on data or constructor args
    m_vertexBuffer = std::make_shared<VertexBuffer>(m_vertices.data(), m_vertices.size() * sizeof(float), hasTexCoords);

    m_vertexArray->addVertexBuffer(m_vertexBuffer);

    m_indexBuffer = std::make_shared<IndexBuffer>(m_indices.data(), m_indices.size());
    m_vertexArray->addIndexBuffer(m_indexBuffer);
}

Mesh::~Mesh() {
    spdlog::debug("Destructor Mesh, indices count: {}, vertices count: {}", m_indices.size(), m_vertices.size());
}

auto Mesh::draw() const -> void {
    m_vertexArray->bind();

    if (m_useIndices) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexBuffer->getCount()), GL_UNSIGNED_INT, nullptr);
    } else {
        // Calculate vertex count based on buffer layout
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(getVertexCount()));
    }
    // Unbind VAO? Usually done after all meshes of the same type are drawn.
    m_vertexArray->unbind();
}

// Helper function implementation
[[nodiscard]] auto Mesh::getVertexCount() const -> size_t {
    if (m_useIndices) {
        // This doesn't make sense for indexed drawing, the count comes from index buffer
        return m_indices.size();  // Or maybe return 0?
    } else {
        if (!m_vertexBuffer)
            return 0;
        // Calculate vertex count based on whether texture coordinates are present in the buffer's layout
        auto floatsPerVertex = m_vertexBuffer->hasTexCoords() ? 5 : 3;
        if (floatsPerVertex == 0)
            return 0;  // Avoid division by zero
        return static_cast<size_t>(m_vertexBuffer->getSize() / (floatsPerVertex * sizeof(float)));
    }
}

}  // namespace Vengine