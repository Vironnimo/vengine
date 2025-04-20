#pragma once

#include <spdlog/spdlog.h>

#include <bitset>
#include <memory>
#include <unordered_map>

#include "components.hpp"
#include "base_system.hpp"
#include "systems.hpp"
#include "vengine/ecs/entities.hpp"

namespace Vengine {

using EntityId = uint64_t;
using ComponentBitset = std::bitset<32>;
// adding component to bitmask:
// entityComponentMask[entityID] |= RenderComponent;
// to check if an entity has a component:
// if (entityComponentMask[entityID] & RenderComponent) {
// Entity has a RenderComponent
// }

class ECS {
   public:
    ECS() {
        m_entities = std::make_shared<Entities>();
        spdlog::debug("Constructor ECS");
    }

    auto createEntity() -> EntityId {
        return m_entities->createEntity();
    }

    auto destroyEntity(EntityId entity) -> void {
        m_entities->destroyEntity(entity);
    }

    template <typename T>
    auto registerComponentType(ComponentType componentType) -> void {
        m_entities->registerComponentType<T>(componentType);
    }

    auto addComponent(EntityId entity, ComponentType componentType) -> void {
        m_entities->addComponent(entity, componentType);
    }

    template <typename T>
    auto getEntityComponent(EntityId entity, ComponentType componentType) -> std::shared_ptr<T> {
        return m_entities->getEntityComponent<T>(entity, componentType);
    }

    auto registerSystem(std::string id, std::shared_ptr<BaseSystem> system) -> void {
        m_systems.emplace(id, std::move(system));
    }

    auto runSystems(float deltaTime) -> void {
        for (const auto& [id, system] : m_systems) {
            system->update(m_entities, deltaTime);
        }
    }

   private:
    std::unordered_map<std::string, std::shared_ptr<BaseSystem>> m_systems;
    std::shared_ptr<Entities> m_entities;
};

}  // namespace Vengine