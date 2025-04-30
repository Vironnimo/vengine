// #include <doctest.h>

// #include <iostream>

// #include "vengine/ecs/components.hpp"
// #include "vengine/ecs/ecs.hpp"
// #include "vengine/ecs/systems.hpp"

// // NOLINTNEXTLINE(readability-function-cognitive-complexity)
// TEST_CASE("ECS Entity Management") {
//     Vengine::ECS ecs;
    

//     SUBCASE("Entity Creation") {
//         auto entity = ecs.createEntity();
//         CHECK(entity > 0);
//     }
    
//     SUBCASE("Multiple Entity Creation") {
//         auto entity1 = ecs.createEntity();
//         auto entity2 = ecs.createEntity();
//         CHECK(entity1 != entity2);
//     }

//     SUBCASE("Component Registration") {
//         auto entity = ecs.createEntity();
//         ecs.addComponent<Vengine::PositionComponent>(entity, Vengine::ComponentType::PositionBit);
        
//         auto position = ecs.getEntityComponent<Vengine::PositionComponent>(entity, Vengine::ComponentType::PositionBit);
//         CHECK(position != nullptr);
        

//         ecs.addComponent<Vengine::VelocityComponent>(entity, Vengine::ComponentType::VelocityBit);
        
//         auto velocity = ecs.getEntityComponent<Vengine::VelocityComponent>(entity, Vengine::ComponentType::VelocityBit);
//         CHECK(velocity != nullptr);
//     }

//     SUBCASE("System Registration and Execution") {
//         auto entity = ecs.createEntity();
//         ecs.addComponent<Vengine::PositionComponent>(entity, Vengine::ComponentType::PositionBit);
//         ecs.addComponent<Vengine::VelocityComponent>(entity, Vengine::ComponentType::VelocityBit);
        
//         auto movementSystem = std::make_shared<Vengine::MovementSystem>();
//         ecs.registerSystem("MovementSystem", movementSystem);
        

//         auto velocity = ecs.getEntityComponent<Vengine::VelocityComponent>(entity, Vengine::ComponentType::VelocityBit);
//         velocity->dx = 1.0f;
//         velocity->dy = 1.0f;

//         ecs.runSystems(1.0f);  
        
//         auto position = ecs.getEntityComponent<Vengine::PositionComponent>(entity, Vengine::ComponentType::PositionBit);
//         CHECK(position->x == 1.0f);  
//     }
// }