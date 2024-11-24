#pragma once

// #include "ecs.hpp"
#include <fmt/core.h>

#include "components.hpp"
#include "ecs.hpp"
#include "base_system.hpp"

namespace Vengine {

class MovementSystem : public BaseSystem {
   public:
    void update(ECS* ecs, float deltaTime) override {
        auto position = ecs->getEntityComponent<PositionComponent>(1, ComponentType::Position);
        auto velocity = ecs->getEntityComponent<VelocityComponent>(1, ComponentType::Velocity);
        fmt::print("Position X: {}\n", position->x);
        fmt::print("Velocity Y: {}\n", velocity->dy);
    }
};

}