#include "mesh.hpp"

#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <cstddef>

#include "vengine/renderer/vertex_array.hpp"

namespace Vengine {

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, VertexLayout layout)
    : m_vertices(vertices), m_indices(indices), m_useIndices(!indices.empty()), m_layout(layout) {
}

auto Mesh::load(const std::string& fileName) -> bool {
    int floatsPerVertex = getFloatsPerVertex();
    // spdlog::debug("Constructor Mesh. Indices: {}, Vertices: {}, Layout: (Pos:{}, Tex:{}, Norm:{}), FloatsPerVertex: {}",
    //               m_indices.size(),
    //               m_vertices.size() / static_cast<size_t>(floatsPerVertex),
    //               m_layout.hasPosition,
    //               m_layout.hasTexCoords,
    //               m_layout.hasNormals,
    //               floatsPerVertex);

    if (m_vertices.empty() || (m_vertices.size() % static_cast<size_t>(floatsPerVertex) != 0)) {
        spdlog::error("Mesh vertex data size ({}) is not a multiple of floatsPerVertex ({})",
                      m_vertices.size(),
                      floatsPerVertex);
    }

    m_needsMainThreadInit = true;
    return true;
}

auto Mesh::finalizeOnMainThread() -> bool {
    // Validate input data
    if (m_vertices.empty()) {
        spdlog::error("Cannot finalize mesh: no vertex data");
        return false;
    }

    // Create vertex buffer
    m_vertexBuffer = std::make_shared<VertexBuffer>(m_vertices.data(), m_vertices.size() * sizeof(float));
    if (!m_vertexBuffer || m_vertexBuffer->getId() == 0) {
        spdlog::error("Failed to create vertex buffer");
        return false;
    }

    // Create vertex array
    m_vertexArray = std::make_shared<VertexArray>();
    if (!m_vertexArray || m_vertexArray->getID() == 0) {
        spdlog::error("Failed to create vertex array");
        return false;
    }
    
    // Add buffer to vertex array
    m_vertexArray->addVertexBuffer(m_vertexBuffer, m_layout);
    
    // Create index buffer if needed
    if (m_useIndices && !m_indices.empty()) {
        m_indexBuffer = std::make_shared<IndexBuffer>(m_indices.data(), m_indices.size());
        m_vertexArray->addIndexBuffer(m_indexBuffer);
    } else {
        spdlog::warn("Mesh created without indices.");
    }  

    m_needsMainThreadInit = false;
    
    // Log successful initialization
    int floatsPerVertex = getFloatsPerVertex();
    spdlog::debug("Mesh finalized successfully: VAO ID: {}, Vertices: {}, Indices: {}",
                m_vertexArray->getID(), 
                m_vertices.size() / static_cast<size_t>(floatsPerVertex),
                m_indices.size());
                
    return true;
}

auto Mesh::unload() -> bool {
    m_vertexArray.reset();
    m_vertexBuffer.reset();
    m_indexBuffer.reset();

    return true;
}

Mesh::~Mesh() {
    // spdlog::debug("Destructor Mesh, indices count: {}, vertices count: {}", m_indices.size(), m_vertices.size());
}

[[nodiscard]] auto Mesh::getFloatsPerVertex() const -> int {
    int count = 0;

    if (m_layout.hasPosition) {
        count += 3;
    }
    if (m_layout.hasTexCoords) {
        count += 2;
    }
    if (m_layout.hasNormals) {
        count += 3;
    }

    return count;
}

[[nodiscard]] auto Mesh::getVertexCount() const -> size_t {
    int floatsPerVertex = getFloatsPerVertex();
    if (floatsPerVertex == 0) {
        return 0;
    }
    return m_vertices.size() / static_cast<size_t>(floatsPerVertex);
}

[[nodiscard]] auto Mesh::getBounds() const -> std::pair<glm::vec3, glm::vec3> {
    if (m_vertices.empty() || !m_layout.hasPosition) {
        return {glm::vec3(0.0f), glm::vec3(0.0f)};
    }

    glm::vec3 min(m_vertices[0], m_vertices[1], m_vertices[2]);
    glm::vec3 max = min;
    int floatsPerVertex = getFloatsPerVertex();

    for (size_t i = 0; i < m_vertices.size(); i += static_cast<size_t>(floatsPerVertex)) {
        glm::vec3 v(m_vertices[i], m_vertices[i + 1], m_vertices[i + 2]);
        min = glm::min(min, v);
        max = glm::max(max, v);
    }
    return {min, max};
}

}  // namespace Vengine