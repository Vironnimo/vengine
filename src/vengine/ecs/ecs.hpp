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
    std::shared_ptr<Entities> entities;

    ECS() {
        entities = std::make_shared<Entities>();
        spdlog::debug("Constructor ECS");
    }

    auto createEntity() const -> EntityId {
        return entities->createEntity();
    }

    auto destroyEntity(EntityId entity) const -> void {
        entities->destroyEntity(entity);
    }

    template <typename T, typename... Args>
    auto addComponent(EntityId entity, ComponentType componentType, Args&&... args) -> void {
        entities->addComponent<T>(entity, componentType, std::forward<Args>(args)...);
    }

    template <typename T>
    auto getEntityComponent(EntityId entity, ComponentType componentType) -> std::shared_ptr<T> {
        return entities->getEntityComponent<T>(entity, componentType);
    }

    auto registerSystem(std::string id, std::shared_ptr<BaseSystem> system) -> void {
        m_systems.emplace(id, std::move(system));
    }

    template <typename T>
    auto getSystem(const std::string& id) -> std::shared_ptr<T> {
        auto it = m_systems.find(id);
        if (it != m_systems.end()) {
            return std::dynamic_pointer_cast<T>(it->second);
        }

        return nullptr;
    }

    auto runSystems(float deltaTime) -> void {
        for (const auto& [id, system] : m_systems) {
            if (!system->isEnabled()) {
                continue;
            }

            system->update(entities, deltaTime);
        }
    }

   private:
    std::unordered_map<std::string, std::shared_ptr<BaseSystem>> m_systems;
};

}  // namespace Vengine