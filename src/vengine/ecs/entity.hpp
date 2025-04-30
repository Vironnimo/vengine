#pragma once

#include <cstdint>
#include <memory>
#include "component_registry.hpp"

namespace Vengine {

class Entities;
using EntityId = uint64_t;

class Entity {
public:
    Entity(EntityId id, Entities* manager);
    Entity();
    
    ~Entity() = default;
    
    [[nodiscard]] auto getId() const -> EntityId;
    [[nodiscard]] auto isValid() const -> bool;
    
    template <typename T, typename... Args>
    auto addComponent(Args&&... args) -> void;
    
    template <typename T>
    auto getComponent() -> std::shared_ptr<T>;
    
    template <typename T>
    auto hasComponent() -> bool;
    
    template <typename T>
    auto removeComponent() -> void;
    
    auto destroy() -> void;
    
private:
    EntityId m_id = 0;
    ComponentBitset m_componentBitset;
    Entities* m_manager = nullptr;
};

} // namespace Vengine
#include "entity_impl.hpp"