#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "component_registry.hpp"
#include "vengine/core/uuid.hpp"
#include "vengine/ecs/components.hpp"

namespace Vengine {

class Entity;
using EntityId = uint64_t;
using ComponentBitset = std::bitset<32>;

class Entities {
   public:
    Entities(std::shared_ptr<ComponentRegistry> registry) : m_registry(std::move(registry)) {
    }

    auto createEntity() -> EntityId {
        EntityId entity = UUID::create();
        m_entityComponents[entity] = ComponentBitset();
        m_entities.insert(entity);
        return entity;
    }

    auto destroyEntity(EntityId entity) -> void {
        m_entityComponents.erase(entity);
        m_entities.erase(entity);

        for (uint32_t i = 0; i < m_registry->size(); i++) {
            auto it = m_components.find(i);
            if (it != m_components.end()) {
                it->second.erase(entity);
            }
        }
    }

    template <typename T, typename... Args>
    auto addComponent(EntityId entity, Args&&... args) -> void {
        ComponentId id = m_registry->getComponentId<T>();

        if (m_entities.find(entity) == m_entities.end()) {
            return;
        }

        m_entityComponents[entity].set(id);
        m_components[id][entity] = std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    auto getEntityComponent(EntityId entity) -> std::shared_ptr<T> {
        ComponentId id = m_registry->getComponentId<T>();

        auto compIt = m_components.find(id);
        if (compIt == m_components.end()) {
            return nullptr;
        }

        auto entityIt = compIt->second.find(entity);
        if (entityIt == compIt->second.end()) {
            return nullptr;
        }

        return std::static_pointer_cast<T>(entityIt->second);
    }

    template <typename T>
    auto hasComponent(EntityId entity) -> bool {
        ComponentId id = m_registry->getComponentId<T>();
        auto it = m_entityComponents.find(entity);
        if (it != m_entityComponents.end()) {
            return it->second.test(id);
        }
        return false;
    }

    template <typename T>
    auto removeComponent(EntityId entity) -> void {
        ComponentId id = m_registry->getComponentId<T>();

        auto it = m_entityComponents.find(entity);
        if (it != m_entityComponents.end()) {
            it->second.reset(id);
        }

        auto compIt = m_components.find(id);
        if (compIt != m_components.end()) {
            compIt->second.erase(entity);
        }
    }

    template <typename... Ts>
    auto getEntitiesWith() -> std::vector<EntityId> {
        if constexpr (sizeof...(Ts) == 0) {
            return {};
        } else {
            ComponentBitset mask;
            (mask.set(m_registry->getComponentId<Ts>()), ...);

            std::vector<EntityId> result;
            for (auto& [entityId, components] : m_entityComponents) {
                if ((components & mask) == mask) {
                    result.push_back(entityId);
                }
            }
            return result;
        }
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
    std::shared_ptr<ComponentRegistry> m_registry;
    std::unordered_set<EntityId> m_entities;
    std::unordered_map<ComponentId, std::unordered_map<EntityId, std::shared_ptr<BaseComponent>>> m_components;
    std::unordered_map<EntityId, ComponentBitset> m_entityComponents;
};

}  // namespace Vengine