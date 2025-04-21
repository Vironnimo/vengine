#include <doctest.h>
#include <iostream>

#include "vengine/ecs/entities.hpp"
#include "vengine/ecs/components.hpp"

using namespace Vengine;

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Entity Creation and Destruction") {
    Entities entities;
    
    SUBCASE("Entity creation") {
        auto entity = entities.createEntity();
        CHECK(entities.getEntityCount() == 1);
    }
    
    SUBCASE("Entity destruction") {
        auto entity = entities.createEntity();
        entities.destroyEntity(entity);
        CHECK(entities.getEntityCount() == 0);
    }
    
    SUBCASE("Multiple entities") {
        auto entity1 = entities.createEntity();
        auto entity2 = entities.createEntity();
        auto entity3 = entities.createEntity();
        CHECK(entities.getEntityCount() == 3);
        
        entities.destroyEntity(entity2);
        CHECK(entities.getEntityCount() == 2);
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Component Management") {
    Entities entities;
    
    SUBCASE("Add and check components") {
        auto entity = entities.createEntity();
        entities.addComponent<PositionComponent>(entity, ComponentType::PositionBit);
        
        CHECK(entities.hasComponent(entity, ComponentType::PositionBit) == true);
        CHECK(entities.hasComponent(entity, ComponentType::VelocityBit) == false);
    }
    
    SUBCASE("Get component data") {
        auto entity = entities.createEntity();
        entities.addComponent<PositionComponent>(entity, ComponentType::PositionBit);
        
        auto component = entities.getEntityComponent<PositionComponent>(entity, ComponentType::PositionBit);
        REQUIRE(component != nullptr);
        component->x = 42;
        
        auto retrievedComponent = entities.getEntityComponent<PositionComponent>(entity, ComponentType::PositionBit);
        CHECK(retrievedComponent->x == 42);
    }
    
    SUBCASE("Component removal on entity destruction") {
        auto entity = entities.createEntity();
        entities.addComponent<PositionComponent>(entity, ComponentType::PositionBit);
        entities.addComponent<VelocityComponent>(entity, ComponentType::VelocityBit);
        
        entities.destroyEntity(entity);
        
        // Entity no longer exists, so it shouldn't have components
        CHECK(entities.hasComponent(entity, ComponentType::PositionBit) == false);
        CHECK(entities.hasComponent(entity, ComponentType::VelocityBit) == false);
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Entity Queries") {
    Entities entities;
        
    SUBCASE("Query entities with components") {
        auto entity1 = entities.createEntity();
        auto entity2 = entities.createEntity();
        auto entity3 = entities.createEntity();
        
        entities.addComponent<PositionComponent>(entity1, ComponentType::PositionBit);
        entities.addComponent<VelocityComponent>(entity1, ComponentType::VelocityBit);
        
        entities.addComponent<PositionComponent>(entity2, ComponentType::PositionBit);
        
        entities.addComponent<VelocityComponent>(entity3, ComponentType::VelocityBit);
        
        // Entities with component 1
        auto entitiesWithComp1 = entities.getEntitiesWith(ComponentType::PositionBit);
        CHECK(entitiesWithComp1.size() == 2);
        CHECK((entitiesWithComp1[0] == entity1 || entitiesWithComp1[0] == entity2));
        CHECK((entitiesWithComp1[1] == entity1 || entitiesWithComp1[1] == entity2));
        
        // Entities with component 2
        auto entitiesWithComp2 = entities.getEntitiesWith(ComponentType::VelocityBit);
        CHECK(entitiesWithComp2.size() == 2);
        CHECK((entitiesWithComp2[0] == entity1 || entitiesWithComp2[0] == entity3));
        CHECK((entitiesWithComp2[1] == entity1 || entitiesWithComp2[1] == entity3));
        
        // Entities with both components
        auto entitiesWithBoth = entities.getEntitiesWith(ComponentType::PositionBit, ComponentType::VelocityBit);
        CHECK(entitiesWithBoth.size() == 1);
        CHECK(entitiesWithBoth[0] == entity1);
        
        // Entities with non-existent component
        // auto entitiesWithComp3 = entities.getEntitiesWith(ComponentType::None);
        // CHECK(entitiesWithComp3.empty());
    }
    
    SUBCASE("Entity reference") {
        auto entityId = entities.createEntity();
        entities.addComponent<PositionComponent>(entityId, ComponentType::PositionBit);
        
        Entity entity = entities.getEntity(entityId);
        CHECK(entity.getId() == entityId);
    }
}