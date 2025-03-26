#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "vertex_array.hpp"
#include "vertex_buffer.hpp"

namespace Vengine {

// todo should become a component for the ecs someday
// todo all the transform should be it's own component/system
class Mesh {
   public:
    Mesh(const std::vector<float>& vertices);
    Mesh(const std::vector<float>& vertices, const std::vector<uint32_t>& indices);

    // transform methods
    auto setPosition(const glm::vec3& position) -> void;
    auto setRotation(float angleInRadians, const glm::vec3& axis) -> void;
    auto setScale(const glm::vec3& scale) -> void;
    // get combined transform matrix
    [[nodiscard]] auto getTransform() const -> glm::mat4;

    auto draw() const -> void;

   private:
    std::vector<float> m_vertices;
    std::vector<uint32_t> m_indices;
    std::shared_ptr<VertexArray> m_vertexArray;
    std::shared_ptr<VertexBuffer> m_vertexBuffer;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
    bool m_useIndices = false;

    // transform stuff
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f);
    glm::mat4 m_transform = glm::mat4(1.0f);

    // functions
    auto updateTransform() -> void;
};

}  // namespace Vengine