#pragma once

#include <spdlog/spdlog.h>
#include "entities.hpp"

#include "components.hpp"
#include "base_system.hpp"
#include "systems/script_system.hpp"

namespace Vengine {

class TransformSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float /*deltaTime*/) override {
        auto list = entities->getEntitiesWith<TransformComponent>();
        for (auto entityId : list) {
            auto transform = entities->getEntityComponent<TransformComponent>(entityId);
            if (transform && transform->dirty) {
                transform->updateMatrix();
                transform->dirty = false;
            }
        }
    }
};

class PhysicsSystem : public BaseSystem {
   public:
    float gravityAcceleration = -9.81f;

    void update(std::shared_ptr<Entities> entities, float deltaTime) override {
        auto list = entities->getEntitiesWith<TransformComponent, RigidbodyComponent>();

        for (auto entityId : list) {
            auto transformComp = entities->getEntityComponent<TransformComponent>(entityId);
            auto rigidbodyComp = entities->getEntityComponent<RigidbodyComponent>(entityId);

            // TODO remove the isGrounded check, it should be handled elsewhere
            if (rigidbodyComp->isStatic || rigidbodyComp->isGrounded) {
                continue;
            }

            if (rigidbodyComp->useGravity) {
                rigidbodyComp->velocity.y += gravityAcceleration * deltaTime;
            }

            // move object
            glm::vec3 newPos = transformComp->getPosition() + rigidbodyComp->velocity * deltaTime;
            transformComp->setPosition(newPos.x, newPos.y, newPos.z);
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
            colliderA->updateWorldBounds(transformA->getTransform());

            for (size_t j = i + 1; j < collidables.size(); ++j) {
                EntityId entityBId = collidables[j];
                auto transformB = entities->getEntityComponent<TransformComponent>(entityBId);
                auto colliderB = entities->getEntityComponent<ColliderComponent>(entityBId);

                if (!transformB || !colliderB) {
                    continue;
                }

                colliderB->updateWorldBounds(transformB->getTransform());

                // AABB
                bool collisionX =
                    colliderA->worldMax.x >= colliderB->worldMin.x && colliderA->worldMin.x <= colliderB->worldMax.x;
                bool collisionY =
                    colliderA->worldMax.y >= colliderB->worldMin.y && colliderA->worldMin.y <= colliderB->worldMax.y;
                bool collisionZ =
                    colliderA->worldMax.z >= colliderB->worldMin.z && colliderA->worldMin.z <= colliderB->worldMax.z;

                if (collisionX && collisionY && collisionZ) {
                    colliderA->colliding = true;
                    colliderB->colliding = true;
                    colliderA->collidingWith = entityBId;
                    colliderB->collidingWith = entityAId;

                    // TODO remove this here, should be handled elsewhere and better
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