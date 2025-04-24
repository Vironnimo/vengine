#include "mesh.hpp"

#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace Vengine {

Mesh::Mesh(const std::vector<float>& vertices) : m_vertices(vertices) {
    spdlog::debug("Constructor Mesh without indices, vertices count: {}", m_vertices.size());
    m_vertexArray = std::make_shared<VertexArray>();

    bool hasTexCoords = false;
    m_vertexBuffer = std::make_shared<VertexBuffer>(m_vertices.data(), m_vertices.size() * sizeof(float), hasTexCoords);

    m_vertexArray->addVertexBuffer(m_vertexBuffer);
}

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
    : m_vertices(vertices), m_indices(indices), m_useIndices(true) {
    spdlog::debug("Constructor Mesh with indices, indices count: {}, vertices count: {}", m_indices.size(), m_vertices.size());
    m_vertexArray = std::make_shared<VertexArray>();
    bool hasTexCoords = true;
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
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(getVertexCount()));
    }
    m_vertexArray->unbind();
}

[[nodiscard]] auto Mesh::getVertexCount() const -> size_t {
    if (m_useIndices) {
        return m_indices.size();
    }

    if (!m_vertexBuffer) {
        return 0;
    }

    auto floatsPerVertex = m_vertexBuffer->hasTexCoords() ? 5 : 3;
    if (floatsPerVertex == 0) {
        return 0;
    }

    return static_cast<size_t>(m_vertexBuffer->getSize() / (floatsPerVertex * sizeof(float)));
}

[[nodiscard]] auto Mesh::getBounds() const -> std::pair<glm::vec3, glm::vec3> {
    if (m_vertices.empty()) {
        return {glm::vec3(0.0f), glm::vec3(0.0f)};
    }
    glm::vec3 min(m_vertices[0], m_vertices[1], m_vertices[2]);
    glm::vec3 max = min;
    for (size_t i = 0; i + 2 < m_vertices.size(); i += 3) {
        glm::vec3 v(m_vertices[i], m_vertices[i + 1], m_vertices[i + 2]);
        min = glm::min(min, v);
        max = glm::max(max, v);
    }
    return {min, max};
}

}  // namespace Vengine