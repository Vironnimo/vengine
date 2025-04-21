#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <memory>

#include "vengine/renderer/material.hpp"
#include "vengine/renderer/mesh.hpp"

namespace Vengine {

enum ComponentType {
    PositionBit = 1 << 0,
    VelocityBit = 1 << 1,
    MeshBit = 1 << 2,
    TransformBit = 1 << 3,
    MaterialBit = 1 << 4,
};

struct BaseComponent {
    virtual ~BaseComponent() = default;
};

struct PositionComponent : public BaseComponent {
    float x = 0.0f;
    float y = 0.0f;
};

struct VelocityComponent : public BaseComponent {
    float dy = 0.0f;
    float dx = 0.0f;
};

struct MeshComponent : public BaseComponent {
    MeshComponent(std::shared_ptr<Mesh> mesh) : mesh(std::move(mesh)) {
    }

    std::shared_ptr<Mesh> mesh;
};

struct MaterialComponent : public BaseComponent {
    MaterialComponent(std::shared_ptr<Material> material) : material(std::move(material)) {
    }

    std::shared_ptr<Material> material;
};

struct TransformComponent : public BaseComponent {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); 
    glm::vec3 scale = glm::vec3(1.0f);
    glm::mat4 transform = glm::mat4(1.0f);

    // Helper to update the matrix (could be moved to a TransformSystem later)
    void updateMatrix() {
        transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, scale);
    }
};

}  // namespace Vengine