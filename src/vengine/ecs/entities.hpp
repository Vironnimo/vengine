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
#include "vengine/ecs/entity.hpp"

namespace Vengine {

// class Entity;
using EntityId = uint64_t;
using ComponentBitset = std::bitset<32>;

class Entities {
   public:
    Entities(std::shared_ptr<ComponentRegistry> registry) : m_registry(std::move(registry)) {
        spdlog::debug("Constructor Entities");
    }

    ~Entities() {
        spdlog::debug("Destructor Entities");
        clear();
    }

    auto createEntity() -> EntityId {
        EntityId entity = UUID::create();
        m_entityBitsets[entity] = ComponentBitset();
        m_entities.insert(entity);
        return entity;
    }

    auto getEntity(EntityId entity) -> Entity {
        return {entity, this};
    }

    auto getEntityByTag(const std::string& tag) -> Entity {
        auto taggedEntities = getEntitiesWith<TagComponent>();

        for (auto entityId : taggedEntities) {
            auto tagComponent = getEntityComponent<TagComponent>(entityId);
            if (tagComponent && tagComponent->tag == tag) {
                return {entityId, this};
            }
        }

        return {};
    }

    auto destroyEntity(EntityId entity) -> void {
        m_entityBitsets.erase(entity);
        m_entities.erase(entity);
        UUID::free(entity);

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

        m_entityBitsets[entity].set(id);
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
    auto getComponentByEntityTag(const std::string& tag) -> std::shared_ptr<T> {
        auto taggedEntities = getEntitiesWith<TagComponent>();

        for (auto entityId : taggedEntities) {
            auto tagComponent = getEntityComponent<TagComponent>(entityId);
            if (tagComponent && tagComponent->tag == tag) {
                return getEntityComponent<T>(entityId);
            }
        }

        return nullptr;
    }

    template <typename T>
    auto hasComponent(EntityId entity) -> bool {
        ComponentId id = m_registry->getComponentId<T>();
        auto it = m_entityBitsets.find(entity);
        if (it != m_entityBitsets.end()) {
            return it->second.test(id);
        }
        return false;
    }

    template <typename T>
    auto removeComponent(EntityId entity) -> void {
        ComponentId id = m_registry->getComponentId<T>();

        auto it = m_entityBitsets.find(entity);
        if (it != m_entityBitsets.end()) {
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
            result.reserve(m_entities.size()); // bad optimization i guess, at least memory vise
            for (auto& [entityId, components] : m_entityBitsets) {
                if ((components & mask) == mask) {
                    result.push_back(entityId);
                }
            }
            return result;
        }
    }

    auto getEntityCount() -> size_t {
        return m_entityBitsets.size();
    }

    auto clear() -> void {
        // free each entity id in the uuid
        for (const auto& entityId : m_entities) {
            UUID::free(entityId);
        }
        m_entities.clear();
        m_components.clear();
        m_entityBitsets.clear();
    }

    auto removeNonPersistentEntities() -> void {
        ComponentId persistentCompId = m_registry->getComponentId<PersistentComponent>();

        auto entityIt = m_entities.begin();
        while (entityIt != m_entities.end()) {
            EntityId currentEntityId = *entityIt;
            bool isPersistent = false;

            auto bitsetIt = m_entityBitsets.find(currentEntityId);
            if (bitsetIt != m_entityBitsets.end()) {
                isPersistent = bitsetIt->second.test(persistentCompId);
            }

            if (!isPersistent) {
                EntityId idToDestroy = currentEntityId;
                // advance iterator BEFORE erasing the element it points to
                entityIt = m_entities.erase(entityIt);
                // remove bitset
                if (bitsetIt != m_entityBitsets.end()) {
                     m_entityBitsets.erase(bitsetIt);
                }
                // remove components 
                for (uint32_t i = 0; i < m_registry->size(); i++) {
                    auto compMapIt = m_components.find(i);
                    if (compMapIt != m_components.end()) {
                        compMapIt->second.erase(idToDestroy);
                    }
                }

                UUID::free(idToDestroy);
            } else {
                ++entityIt;
            }
        }
    }

   private:
    std::shared_ptr<ComponentRegistry> m_registry;
    std::unordered_set<EntityId> m_entities;
    std::unordered_map<ComponentId, std::unordered_map<EntityId, std::shared_ptr<BaseComponent>>> m_components;
    std::unordered_map<EntityId, ComponentBitset> m_entityBitsets;
};

}  // namespace Vengine