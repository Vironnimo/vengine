#pragma once

#include <spdlog/spdlog.h>

#include "entities.hpp"
#include "components.hpp"
#include "base_system.hpp"
#include "vengine/renderer/camera.hpp"

namespace Vengine {

class MovementSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float deltaTime) override {
        auto list = entities->getEntitiesWith(ComponentType::PositionBit, ComponentType::VelocityBit);
        for (auto entity : list) {
            auto position = entities->getEntityComponent<PositionComponent>(entity, ComponentType::PositionBit);
            auto velocity = entities->getEntityComponent<VelocityComponent>(entity, ComponentType::VelocityBit);
            position->x += velocity->dx * deltaTime;
            position->y += velocity->dy * deltaTime;

            // reset velocity
            velocity->dx = 0.0f;
            velocity->dy = 0.0f;
        }
    }
};

class RenderSystem : public BaseSystem {
   public:
    RenderSystem(std::shared_ptr<Camera> camera) : m_camera(std::move(camera)) {
        if (!m_camera) {
            spdlog::warn("RenderSystem created without a valid Camera!");
        }
    }

    void update(std::shared_ptr<Entities> entities, float /*deltaTime*/) override {
        if (!m_camera) {
            spdlog::error("RenderSystem cannot update without a camera.");
            return;
        }

        // camera matrices
        glm::mat4 viewMatrix = m_camera->getViewMatrix();
        glm::mat4 projectionMatrix = m_camera->getProjectionMatrix();

        auto list =
            entities->getEntitiesWith(ComponentType::TransformBit, ComponentType::MeshBit, ComponentType::MaterialBit);

        for (auto entity : list) {
            auto transformComp = entities->getEntityComponent<TransformComponent>(entity, ComponentType::TransformBit);
            auto meshComp = entities->getEntityComponent<MeshComponent>(entity, ComponentType::MeshBit);
            auto materialComp = entities->getEntityComponent<MaterialComponent>(entity, ComponentType::MaterialBit);

            if (transformComp && meshComp && meshComp->mesh && materialComp && materialComp->material) {
                materialComp->material->bind();

                auto shader = materialComp->material->getShader();
                if (!shader) {
                    spdlog::warn("Material has no shader, skipping entity {}", entity);
                    continue;
                }

                transformComp->updateMatrix();

                shader->setUniformMat4("uView", viewMatrix);
                shader->setUniformMat4("uProjection", projectionMatrix);
                shader->setUniformMat4("uTransform", transformComp->transform);

                meshComp->mesh->draw();
            } else {
                spdlog::warn("Entity {} is missing required components for rendering", entity);
            }
        }
    }

   private:
    std::shared_ptr<Camera> m_camera;
};

class PhysicsSystem : public BaseSystem {
   public:
    float gravityAcceleration = -9.81f;

    void update(std::shared_ptr<Entities> entities, float deltaTime) override {
        auto list = entities->getEntitiesWith(ComponentType::TransformBit, ComponentType::RigidBodyBit);

        for (auto entityId : list) {
            auto transformComp =
                entities->getEntityComponent<TransformComponent>(entityId, ComponentType::TransformBit);
            auto rigidbodyComp =
                entities->getEntityComponent<RigidbodyComponent>(entityId, ComponentType::RigidBodyBit);

            // todo remove the isgrounded check, it should be handled elsewhere 
            if (rigidbodyComp->isStatic || rigidbodyComp->isGrounded) {
                continue;
            }

            if (rigidbodyComp->useGravity) {
                rigidbodyComp->velocity.y += gravityAcceleration * deltaTime;
            }

            // move object
            transformComp->position += rigidbodyComp->velocity * deltaTime;
            transformComp->updateMatrix();
        }
    }
};

class CollisionSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float /*deltaTime*/) override {
        // Get all entities that have both a Transform and a Collider
        auto list = entities->getEntitiesWith(ComponentType::TransformBit, ComponentType::ColliderBit);
        // spdlog::debug("CollisionSystem found {} entities to check for collisions", list.size());
        std::vector<EntityId> collidables(list.begin(), list.end());  // Convert to vector for easier iteration

        // Reset collision state for all colliders
        for (EntityId entityId : collidables) {
            auto colliderComp = entities->getEntityComponent<ColliderComponent>(entityId, ComponentType::ColliderBit);
            if (colliderComp) {
                colliderComp->colliding = false;
                colliderComp->collidingWith = 0;  // Reset colliding entity ID
            }
        }

        // Update world bounds and perform collision checks (N^2 for now, could be optimized)
        for (size_t i = 0; i < collidables.size(); ++i) {
            EntityId entityAId = collidables[i];
            auto transformA = entities->getEntityComponent<TransformComponent>(entityAId, ComponentType::TransformBit);
            auto colliderA = entities->getEntityComponent<ColliderComponent>(entityAId, ComponentType::ColliderBit);

            // Ensure components are valid and update world bounds
            if (!transformA || !colliderA)
                continue;
            colliderA->updateWorldBounds(transformA->transform);  // Assumes transform matrix is up-to-date

            for (size_t j = i + 1; j < collidables.size(); ++j) {
                EntityId entityBId = collidables[j];
                auto transformB =
                    entities->getEntityComponent<TransformComponent>(entityBId, ComponentType::TransformBit);
                auto colliderB = entities->getEntityComponent<ColliderComponent>(entityBId, ComponentType::ColliderBit);

                // Ensure components are valid and update world bounds
                if (!transformB || !colliderB)
                    continue;
                // Only update if not already updated this frame (simple optimization)
                if (colliderB->worldMin == glm::vec3(0.0f) &&
                    colliderB->worldMax == glm::vec3(0.0f)) {  // Check if bounds are default/unset
                    colliderB->updateWorldBounds(transformB->transform);
                }

                // AABB Collision Check
                bool collisionX =
                    colliderA->worldMax.x >= colliderB->worldMin.x && colliderA->worldMin.x <= colliderB->worldMax.x;
                bool collisionY =
                    colliderA->worldMax.y >= colliderB->worldMin.y && colliderA->worldMin.y <= colliderB->worldMax.y;
                bool collisionZ =
                    colliderA->worldMax.z >= colliderB->worldMin.z && colliderA->worldMin.z <= colliderB->worldMax.z;

                if (collisionX && collisionY && collisionZ) {
                    // Collision detected!
                    colliderA->colliding = true;
                    colliderB->colliding = true;
                    colliderA->collidingWith = entityBId;
                    colliderB->collidingWith = entityAId;

                    // set isGrounded if colliding
                    auto rigidbodyA = entities->getEntityComponent<RigidbodyComponent>(entityAId, ComponentType::RigidBodyBit);
                    auto rigidbodyB = entities->getEntityComponent<RigidbodyComponent>(entityBId, ComponentType::RigidBodyBit);
                    rigidbodyA->isGrounded = true;
                    rigidbodyB->isGrounded = true;
                    // Simple logging for now
                    spdlog::debug("Collision detected between entity {} and {}", entityAId, entityBId);

                    // TODO: Implement collision response (e.g., notify other systems, resolve penetration)
                }
            }
            // Reset world bounds after checks for this entity are done to ensure fresh calculation next frame
            // Or manage update state more carefully if needed across systems.
            colliderA->worldMin = glm::vec3(0.0f);
            colliderA->worldMax = glm::vec3(0.0f);
        }
    }
};

}  // namespace Vengine