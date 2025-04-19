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
    
    entities.registerComponentType<PositionComponent>(ComponentType::Position);
    entities.registerComponentType<VelocityComponent>(ComponentType::Velocity);
    
    SUBCASE("Add and check components") {
        auto entity = entities.createEntity();
        entities.addComponent(entity, ComponentType::Position);
        
        CHECK(entities.hasComponent(entity, ComponentType::Position) == true);
        CHECK(entities.hasComponent(entity, ComponentType::Velocity) == false);
    }
    
    SUBCASE("Get component data") {
        auto entity = entities.createEntity();
        entities.addComponent(entity, ComponentType::Position);
        
        auto component = entities.getEntityComponent<PositionComponent>(entity, ComponentType::Position);
        REQUIRE(component != nullptr);
        component->x = 42;
        
        auto retrievedComponent = entities.getEntityComponent<PositionComponent>(entity, ComponentType::Position);
        CHECK(retrievedComponent->x == 42);
    }
    
    SUBCASE("Component removal on entity destruction") {
        auto entity = entities.createEntity();
        entities.addComponent(entity, ComponentType::Position);
        entities.addComponent(entity, ComponentType::Velocity);
        
        entities.destroyEntity(entity);
        
        // Entity no longer exists, so it shouldn't have components
        CHECK(entities.hasComponent(entity, ComponentType::Position) == false);
        CHECK(entities.hasComponent(entity, ComponentType::Velocity) == false);
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_CASE("Entity Queries") {
    Entities entities;
    
    entities.registerComponentType<PositionComponent>(ComponentType::Position);
    entities.registerComponentType<VelocityComponent>(ComponentType::Velocity);
    
    SUBCASE("Query entities with components") {
        auto entity1 = entities.createEntity();
        auto entity2 = entities.createEntity();
        auto entity3 = entities.createEntity();
        
        entities.addComponent(entity1, ComponentType::Position);
        entities.addComponent(entity1, ComponentType::Velocity);
        
        entities.addComponent(entity2, ComponentType::Position);
        
        entities.addComponent(entity3, ComponentType::Velocity);
        
        // Entities with component 1
        auto entitiesWithComp1 = entities.getEntitiesWith(ComponentType::Position);
        CHECK(entitiesWithComp1.size() == 2);
        CHECK((entitiesWithComp1[0] == entity1 || entitiesWithComp1[0] == entity2));
        CHECK((entitiesWithComp1[1] == entity1 || entitiesWithComp1[1] == entity2));
        
        // Entities with component 2
        auto entitiesWithComp2 = entities.getEntitiesWith(ComponentType::Velocity);
        CHECK(entitiesWithComp2.size() == 2);
        CHECK((entitiesWithComp2[0] == entity1 || entitiesWithComp2[0] == entity3));
        CHECK((entitiesWithComp2[1] == entity1 || entitiesWithComp2[1] == entity3));
        
        // Entities with both components
        auto entitiesWithBoth = entities.getEntitiesWith(ComponentType::Position, ComponentType::Velocity);
        CHECK(entitiesWithBoth.size() == 1);
        CHECK(entitiesWithBoth[0] == entity1);
        
        // Entities with non-existent component
        // auto entitiesWithComp3 = entities.getEntitiesWith(ComponentType::None);
        // CHECK(entitiesWithComp3.empty());
    }
    
    SUBCASE("Entity reference") {
        auto entityId = entities.createEntity();
        entities.addComponent(entityId, ComponentType::Position);
        
        Entity entity = entities.getEntity(entityId);
        CHECK(entity.getId() == entityId);
    }
}

