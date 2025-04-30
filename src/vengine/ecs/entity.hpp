#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include "vengine/ecs/components.hpp"

namespace Vengine {

class Entities;

using EntityId = uint64_t;
using ComponentBitset = std::bitset<32>;

class Entity {
   public:
    Entity(EntityId id, Entities* manager);
    Entity(); 

    ~Entity() = default;

    [[nodiscard]] auto getId() const -> EntityId;
    [[nodiscard]] auto isValid() const -> bool;

    template <typename T, typename... Args>
    auto addComponent(ComponentType componentType, Args&&... args) -> void;
    template <typename T>
    auto getComponent(ComponentType componentType) -> std::shared_ptr<T>;
    auto hasComponent(ComponentType componentType) -> bool;
    auto removeComponent(ComponentType componentType) -> void;

    auto destroy() -> void;


   private:
    EntityId m_id = 0; 
    Entities* m_manager = nullptr; 
};

}  // namespace Vengine
#include "vengine/ecs/entity_impl.hpp" // include the implementation file for template methods