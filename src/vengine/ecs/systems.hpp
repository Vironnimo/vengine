#pragma once

#include <spdlog/spdlog.h>

#include "entities.hpp"
#include "components.hpp"
#include "base_system.hpp"

namespace Vengine {

class MovementSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float deltaTime) override {
        auto list = entities->getEntitiesWith(ComponentType::Position, ComponentType::Velocity);
        for (auto entity : list) {
            // position before
            // spdlog::debug("Entity ID: {}", entity);
            // spdlog::debug("Position X: {}", entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position)->x);
            // spdlog::debug("Position Y: {}", entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position)->y);
            // spdlog::debug("Velocity X: {}", entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity)->dx);
            // spdlog::debug("Velocity Y: {}", entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity)->dy);

            auto position = entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position);
            auto velocity = entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity);
            position->x += velocity->dx * deltaTime;
            position->y += velocity->dy * deltaTime;

            velocity->dx = 0.0f;  // reset velocity after applying it to position
            velocity->dy = 0.0f;  // reset velocity after applying it to position

            // spdlog::debug("Entity ID: {}", entity);
            // spdlog::debug("Position X: {}", entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position)->x);
            // spdlog::debug("Position Y: {}", entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position)->y);
            // spdlog::debug("Velocity X: {}", entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity)->dx);
            // spdlog::debug("Velocity Y: {}", entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity)->dy);
        }
    }
};

}  // namespace Vengine