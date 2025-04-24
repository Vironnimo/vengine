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
    RigidBodyBit = 1 << 5,
    ColliderBit = 1 << 6,
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

struct RigidbodyComponent : public BaseComponent {
    glm::vec3 velocity = glm::vec3(0.0f);
    float mass = 1.0f; 
    bool useGravity = true;
    bool isGrounded = false;
    bool isStatic = false; 
};

struct ColliderComponent : public BaseComponent {
    using EntityId = uint64_t;
    // AABB defined by min and max points relative to the entity's origin
    glm::vec3 min = glm::vec3(-0.5f); // Default half-unit cube centered at origin
    glm::vec3 max = glm::vec3(0.5f);

    // Optional: Store world-space AABB after transformation for easier checks
    glm::vec3 worldMin;
    glm::vec3 worldMax;

    // Flag to indicate if a collision occurred this frame
    bool colliding = false;
    EntityId collidingWith = 0; // ID of the entity it's colliding with (simple case)

    // Constructor to set initial bounds (relative to origin)
    ColliderComponent(const glm::vec3& minBounds, const glm::vec3& maxBounds)
        : min(minBounds), max(maxBounds) {}

    // Default constructor
    ColliderComponent() = default;

    // Helper to update world bounds based on transform
    void updateWorldBounds(const glm::mat4& transform) {
         // Transform the 8 corners of the local AABB to world space
        glm::vec4 corners[8] = {
            transform * glm::vec4(min.x, min.y, min.z, 1.0f),
            transform * glm::vec4(max.x, min.y, min.z, 1.0f),
            transform * glm::vec4(min.x, max.y, min.z, 1.0f),
            transform * glm::vec4(min.x, min.y, max.z, 1.0f),
            transform * glm::vec4(max.x, max.y, min.z, 1.0f),
            transform * glm::vec4(min.x, max.y, max.z, 1.0f),
            transform * glm::vec4(max.x, min.y, max.z, 1.0f),
            transform * glm::vec4(max.x, max.y, max.z, 1.0f)
        };

        // Find the min and max extents in world space
        worldMin = glm::vec3(corners[0]);
        worldMax = glm::vec3(corners[0]);
        for (int i = 1; i < 8; ++i) {
            worldMin = glm::min(worldMin, glm::vec3(corners[i]));
            worldMax = glm::max(worldMax, glm::vec3(corners[i]));
        }
    }
};

}  // namespace Vengine