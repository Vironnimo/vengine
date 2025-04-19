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
            auto position = entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position);
            auto velocity = entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity);
            position->x += velocity->dx * deltaTime;
            position->y += velocity->dy * deltaTime;
    
            spdlog::debug("Entity ID: {}\n", entity);
            spdlog::debug("Position X: {}\n", position->x);
            spdlog::debug("Velocity Y: {}\n", velocity->dy);
        }

        // auto position = entities->getEntityComponent<PositionComponent>(1, ComponentType::Position);
        // auto velocity = entities->getEntityComponent<VelocityComponent>(1, ComponentType::Velocity);
    }
};

}  // namespace Vengine