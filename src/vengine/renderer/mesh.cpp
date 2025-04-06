#include "mesh.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Vengine {

Mesh::Mesh(const std::vector<float>& vertices) : m_vertices(vertices) {
    spdlog::debug("Constructor Mesh without indices, vertices count: {}", m_vertices.size());
    m_vertexArray = std::make_shared<VertexArray>();
    m_vertexBuffer = std::make_shared<VertexBuffer>(m_vertices.data(), m_vertices.size() * sizeof(float));

    m_vertexArray->addVertexBuffer(m_vertexBuffer);
}

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
    : m_vertices(vertices), m_indices(indices), m_useIndices(true) {
    spdlog::debug("Constructor Mesh with indices, indices count: {}, vertices count: {}", m_indices.size(), m_vertices.size());
    m_vertexArray = std::make_shared<VertexArray>();
    m_vertexBuffer = std::make_shared<VertexBuffer>(m_vertices.data(), m_vertices.size() * sizeof(float), true);

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
        // Calculate vertex count based on whether texture coordinates are present
        auto floatsPerVertex = m_vertexBuffer->hasTexCoords() ? 5 : 3;
        auto vertexCount = static_cast<GLsizei>(m_vertices.size() / floatsPerVertex);

        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
}

auto Mesh::setPosition(const glm::vec3& position) -> void {
    m_position = position;
    updateTransform();
}

auto Mesh::setRotation(float angle, const glm::vec3& axis) -> void {
    float angleInRadians = glm::radians(angle);
    m_rotation = axis * angleInRadians;
    updateTransform();
}

auto Mesh::setScale(const glm::vec3& scale) -> void {
    m_scale = scale;
    updateTransform();
}

auto Mesh::updateTransform() -> void {
    // reset transform, but why?
    m_transform = glm::mat4(1.0f);
    // apply transformations in order: scale, rotate, translate
    m_transform = glm::translate(m_transform, m_position);

    if (m_rotation.x != 0.0f) {
        m_transform = glm::rotate(m_transform, m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    if (m_rotation.y != 0.0f) {
        m_transform = glm::rotate(m_transform, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if (m_rotation.z != 0.0f) {
        m_transform = glm::rotate(m_transform, m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    m_transform = glm::scale(m_transform, m_scale);
}

[[nodiscard]] auto Mesh::getTransform() const -> glm::mat4 {
    return m_transform;
}

auto Mesh::setTexture(std::shared_ptr<Texture> texture) -> void {
    m_texture = std::move(texture);
}

[[nodiscard]] auto Mesh::getTexture() const -> std::shared_ptr<Texture> {
    return m_texture;
}

[[nodiscard]] auto Mesh::hasTexture() const -> bool {
    return m_texture != nullptr;
}

}  // namespace Vengine