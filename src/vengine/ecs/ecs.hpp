#pragma once

#include <spdlog/spdlog.h>

#include <bitset>
#include <memory>
#include <unordered_map>

#include "components.hpp"
#include "systems.hpp"
#include "base_system.hpp"
#include "vengine/ecs/entities.hpp"
#include "vengine/ecs/entity.hpp"

namespace Vengine {

using EntityId = uint64_t;
using ComponentBitset = std::bitset<32>;

class ECS {
   public:
    ECS() {
        spdlog::debug("Constructor ECS");
        m_componentRegistry = std::make_shared<ComponentRegistry>();
        m_activeEntities = std::make_shared<Entities>(m_componentRegistry);
    }

    template <typename T>
    auto registerComponent(const std::string& name = "") -> ComponentId {
        return m_componentRegistry->registerComponent<T>(name);
    }

    auto createEntity() const -> EntityId {
        return m_activeEntities->createEntity();
    }

    auto destroyEntity(EntityId entity) const -> void {
        m_activeEntities->destroyEntity(entity);
    }

    auto getEntity(EntityId entity) const -> Entity {
        return m_activeEntities->getEntity(entity);
    }

    auto getEntityByTag(const std::string& tag) const -> Entity {
        return m_activeEntities->getEntityByTag(tag);
    }

    template <typename T, typename... Args>
    auto addComponent(EntityId entity, Args&&... args) -> void {
        m_activeEntities->addComponent<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    auto getEntityComponent(EntityId entity) -> std::shared_ptr<T> {
        return m_activeEntities->getEntityComponent<T>(entity);
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

            system->update(m_activeEntities, deltaTime);
        }
    }

    auto getEntityCount() const -> size_t {
        return m_activeEntities->getEntityCount();
    }

    auto setActiveEntities(const std::string& name) -> void {
        m_activeEntities = getEntitySet(name);
    }

    auto createEntitySet(const std::string& name) -> std::shared_ptr<Entities> {
        auto entitySet = std::make_shared<Entities>(m_componentRegistry);
        m_entitySets[name] = entitySet;
        return entitySet;
    }

    [[nodiscard]] auto getActiveEntities() const -> std::shared_ptr<Entities> {
        return m_activeEntities;
    }

    auto getEntitySet(const std::string& name) const -> std::shared_ptr<Entities> {
        auto it = m_entitySets.find(name);
        if (it != m_entitySets.end()) {
            return it->second;
        }
        return nullptr;
    }

   private:
    std::shared_ptr<ComponentRegistry> m_componentRegistry;
    std::shared_ptr<Entities> m_activeEntities;
    std::unordered_map<std::string, std::shared_ptr<Entities>> m_entitySets;
    std::unordered_map<std::string, std::shared_ptr<BaseSystem>> m_systems;
};

}  // namespace Vengine