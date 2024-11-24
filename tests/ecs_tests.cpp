#include <gtest/gtest.h>

#include "vengine/ecs/ecs.hpp"
#include "vengine/ecs/systems.hpp"

TEST(ECSTests, movementSystem) {
    Vengine::ECS ecs;
    auto entity = ecs.createEntity();
    ecs.registerComponentType<Vengine::PositionComponent>(Vengine::ComponentType::Position);
    ecs.registerComponentType<Vengine::VelocityComponent>(Vengine::ComponentType::Velocity);
    ecs.addComponent(entity, Vengine::ComponentType::Position);
    ecs.addComponent(entity, Vengine::ComponentType::Velocity);

    ecs.registerSystem(std::make_shared<Vengine::MovementSystem>());
    ecs.runSystems(1.0f);

    auto position = ecs.getEntityComponent<Vengine::PositionComponent>(entity, Vengine::ComponentType::Position);
    EXPECT_EQ(position->x, 0.0f);
}