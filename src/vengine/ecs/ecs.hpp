#pragma once

#include <fmt/core.h>

#include <bitset>
#include <functional>
#include <memory>
#include <unordered_map>

#include "vengine/core/uuid.hpp"
// #include "systems.hpp"
#include "components.hpp"
#include "base_system.hpp"

namespace Vengine {

using Entity = size_t;
using ComponentBitset = std::bitset<32>;
// adding component to bitmask:
// entityComponentMask[entityID] |= RenderComponent;
// to check if an entity has a component:
// if (entityComponentMask[entityID] & RenderComponent) {
// Entity has a RenderComponent
// }

class ECS {
   public:
    ECS();
    void init();

    Entity createEntity();
    void destroyEntity(Entity entity);

    template <typename T>
    void registerComponentType(ComponentType componentType) {
        m_typeFactories[componentType] = [] { return std::make_shared<T>(); };
    }

    // template <typename T>
    void addComponent(Entity entity, ComponentType componentType) {
        m_entityComponents[entity] |= componentType;
        // create component
        // m_components[componentType][entity] = std::make_shared<T>();
        m_components[componentType][entity] = m_typeFactories.find(componentType)->second();
    }

    template <typename T>
    std::shared_ptr<T> getEntityComponent(Entity entity, ComponentType componentType) {
        return std::dynamic_pointer_cast<T>(m_components[componentType][entity]);
    }

    void registerSystem(std::shared_ptr<BaseSystem> system) {
        m_systems.push_back(std::move(system));
    }

    void runSystems(float deltaTime) {
        for (const auto& system : m_systems) {
            system->update(this, deltaTime);
        }
    }

   private:
    std::unordered_map<Entity, ComponentBitset> m_entityComponents;
    std::unordered_map<ComponentType, std::function<std::shared_ptr<BaseComponent>()>> m_typeFactories;
    std::unordered_map<ComponentType, std::unordered_map<Entity, std::shared_ptr<BaseComponent>>> m_components;

    std::vector<std::shared_ptr<BaseSystem>> m_systems;

};

}  // namespace Vengine