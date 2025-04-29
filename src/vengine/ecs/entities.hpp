#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "vengine/core/uuid.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/ecs/entity.hpp"

namespace Vengine {

using EntityId = uint64_t;
using ComponentBitset = std::bitset<32>;

class Entities {
   public:
    Entities() = default;
    ~Entities() = default;

    auto createEntity() -> EntityId {
        EntityId entity = UUID::create();
        m_entityComponents[entity] = ComponentBitset();
        m_entities.insert(entity);
        return entity;
    }

    auto destroyEntity(EntityId entity) -> void {
        m_entityComponents.erase(entity);
        m_entities.erase(entity);
        for (auto& [componentType, entityMap] : m_components) {
            entityMap.erase(entity);
        }
    }

    template <typename T, typename... Args>
    auto addComponent(EntityId entity, ComponentType componentType, Args&&... args) -> void {
        if (m_entities.find(entity) == m_entities.end()) {
            // error handling?
            return;
        }

        m_entityComponents[entity] |= componentType;
        m_components[componentType][entity] = std::make_shared<T>(std::forward<Args>(args)...);
    }


    template <typename T>
    auto getEntityComponent(EntityId entity, ComponentType componentType) -> std::shared_ptr<T> {
        return std::dynamic_pointer_cast<T>(m_components[componentType][entity]);
    }

    template <typename... Args>
    auto getEntitiesWith(Args... args) -> std::vector<EntityId> {
        std::vector<EntityId> entities;
        ComponentBitset queryMask = ComponentBitset();
        ((queryMask |= ComponentBitset(args)), ...);  

        for (const auto& [entity, components] : m_entityComponents) {
            if ((components & queryMask) == queryMask) {
                entities.push_back(entity);
            }
        }

        return entities;
    }

    auto getEntity(EntityId entityId) -> Entity {
        // todo HEREHREHREHRE
        // fill entity with references to its components
        Entity entity(entityId);
        return entity;  
    }

    auto hasComponent(EntityId entity, ComponentType componentType) -> bool {
        auto it = m_entityComponents.find(entity);
        if (it != m_entityComponents.end()) {
            return (it->second & ComponentBitset(componentType)) == ComponentBitset(componentType);
        }
        return false;
    }

    auto getEntityCount() -> size_t {
        return m_entityComponents.size();
    }

    auto clear() -> void {
        m_entities.clear();
        m_components.clear();
        m_entityComponents.clear();
    }

   private:
    std::unordered_set<EntityId> m_entities;
    std::unordered_map<ComponentType, std::unordered_map<EntityId, std::shared_ptr<BaseComponent>>> m_components;
    std::unordered_map<EntityId, ComponentBitset> m_entityComponents;
};

}  // namespace Vengine