#include <doctest.h>

#include <iostream>

#include "vengine/ecs/ecs.hpp"
#include "vengine/ecs/systems.hpp"

TEST_CASE("ECS Entity Management") {
    Vengine::ECS ecs;
    

    SUBCASE("Entity Creation") {
        auto entity = ecs.createEntity();
        CHECK(entity > 0);
    }
    
    SUBCASE("Multiple Entity Creation") {
        auto entity1 = ecs.createEntity();
        auto entity2 = ecs.createEntity();
        CHECK(entity1 != entity2);
    }

    SUBCASE("Component Registration") {
        ecs.registerComponentType<Vengine::PositionComponent>(Vengine::ComponentType::Position);
        ecs.registerComponentType<Vengine::VelocityComponent>(Vengine::ComponentType::Velocity);
        
        auto entity = ecs.createEntity();
        ecs.addComponent(entity, Vengine::ComponentType::Position);
        
        auto position = ecs.getEntityComponent<Vengine::PositionComponent>(entity, Vengine::ComponentType::Position);
        CHECK(position != nullptr);
    }

    SUBCASE("System Registration and Execution") {
        ecs.registerComponentType<Vengine::PositionComponent>(Vengine::ComponentType::Position);
        ecs.registerComponentType<Vengine::VelocityComponent>(Vengine::ComponentType::Velocity);
        
        auto entity = ecs.createEntity();
        ecs.addComponent(entity, Vengine::ComponentType::Position);
        ecs.addComponent(entity, Vengine::ComponentType::Velocity);
        
        auto movementSystem = std::make_shared<Vengine::MovementSystem>();
        ecs.registerSystem("MovementSystem", movementSystem);
        
        ecs.runSystems(1.0f);  // Simulate a frame update
        
        auto position = ecs.getEntityComponent<Vengine::PositionComponent>(entity, Vengine::ComponentType::Position);
        CHECK(position->x == 1.0f);  // Assuming initial position was 0.0f and velocity is 1.0f
    }
}