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
        auto list = entities->getEntitiesWith<PositionComponent, VelocityComponent>();
        for (auto entity : list) {
            auto position = entities->getEntityComponent<PositionComponent>(entity);
            auto velocity = entities->getEntityComponent<VelocityComponent>(entity);
            position->x += velocity->dx * deltaTime;
            position->y += velocity->dy * deltaTime;

            // reset velocity
            velocity->dx = 0.0f;
            velocity->dy = 0.0f;
        }
    }
};

class TransformSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float /*deltaTime*/) override {
        auto list = entities->getEntitiesWith<TransformComponent>();
        for (auto entityId : list) {
            auto transform = entities->getEntityComponent<TransformComponent>(entityId);
            if (transform) {
                transform->updateMatrix();
            }
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

        auto list = entities->getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();

        for (auto entity : list) {
            auto transformComp = entities->getEntityComponent<TransformComponent>(entity);
            auto meshComp = entities->getEntityComponent<MeshComponent>(entity);
            auto materialComp = entities->getEntityComponent<MaterialComponent>(entity);

            if (transformComp && meshComp && meshComp->mesh && materialComp && materialComp->material) {
                materialComp->material->bind();

                auto shader = materialComp->material->getShader();
                if (!shader) {
                    spdlog::warn("Material has no shader, skipping entity {}", entity);
                    continue;
                }

                // TODO: should this be in the material as references?
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
        auto list = entities->getEntitiesWith<TransformComponent, RigidbodyComponent>();

        for (auto entityId : list) {
            auto transformComp = entities->getEntityComponent<TransformComponent>(entityId);
            auto rigidbodyComp = entities->getEntityComponent<RigidbodyComponent>(entityId);

            // todo remove the isGrounded check, it should be handled elsewhere
            if (rigidbodyComp->isStatic || rigidbodyComp->isGrounded) {
                continue;
            }

            if (rigidbodyComp->useGravity) {
                rigidbodyComp->velocity.y += gravityAcceleration * deltaTime;
            }

            // move object
            transformComp->position += rigidbodyComp->velocity * deltaTime;
        }
    }
};

class CollisionSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float /*deltaTime*/) override {
        auto collidables = entities->getEntitiesWith<TransformComponent, ColliderComponent>();

        // do we need to reset colliders? probably yes.
        // for (EntityId entityId : collidables) {
        //     auto colliderComp = entities->getEntityComponent<ColliderComponent>(entityId,
        //     ComponentType::ColliderBit); if (colliderComp) {
        //         colliderComp->colliding = false;
        //         colliderComp->collidingWith = 0;
        //     }
        // }

        // inefficient, O(n^2) collision detection
        for (size_t i = 0; i < collidables.size(); ++i) {
            EntityId entityAId = collidables[i];
            auto transformA = entities->getEntityComponent<TransformComponent>(entityAId);
            auto colliderA = entities->getEntityComponent<ColliderComponent>(entityAId);

            if (!transformA || !colliderA) {
                continue;
            }
            colliderA->updateWorldBounds(transformA->transform);

            for (size_t j = i + 1; j < collidables.size(); ++j) {
                EntityId entityBId = collidables[j];
                auto transformB = entities->getEntityComponent<TransformComponent>(entityBId);
                auto colliderB = entities->getEntityComponent<ColliderComponent>(entityBId);

                if (!transformB || !colliderB) {
                    continue;
                }

                colliderB->updateWorldBounds(transformB->transform);

                // AABB
                bool collisionX = colliderA->worldMax.x >= colliderB->worldMin.x && colliderA->worldMin.x <= colliderB->worldMax.x;
                bool collisionY = colliderA->worldMax.y >= colliderB->worldMin.y && colliderA->worldMin.y <= colliderB->worldMax.y;
                bool collisionZ = colliderA->worldMax.z >= colliderB->worldMin.z && colliderA->worldMin.z <= colliderB->worldMax.z;

                if (collisionX && collisionY && collisionZ) {
                    colliderA->colliding = true;
                    colliderB->colliding = true;
                    colliderA->collidingWith = entityBId;
                    colliderB->collidingWith = entityAId;

                    // todo remove this here, should be handled elsewhere and better
                    auto rigidbodyA = entities->getEntityComponent<RigidbodyComponent>(entityAId);
                    auto rigidbodyB = entities->getEntityComponent<RigidbodyComponent>(entityBId);
                    rigidbodyA->isGrounded = true;
                    rigidbodyB->isGrounded = true;
                }
            }

            // reset even neccesary?
            colliderA->worldMin = glm::vec3(0.0f);
            colliderA->worldMax = glm::vec3(0.0f);
        }
    }
};

}  // namespace Vengine