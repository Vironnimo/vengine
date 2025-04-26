#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include "vengine/ecs/components.hpp"

namespace Vengine {

using EntityId = uint64_t;
using ComponentBitset = std::bitset<32>;

class Entity {
   public:
    Entity(EntityId id) : m_id(id) {
    }

    ~Entity() = default;

    [[nodiscard]] auto getId() const -> EntityId {
        return m_id;
    }

    auto setComponent(ComponentType componentType, std::shared_ptr<BaseComponent> component) -> void {
        m_components[componentType] = std::move(component);
    }

    auto addComponent(ComponentType componentType) -> void {
        // m_components[componentType] = std::make_shared<BaseComponent>();
    }

    auto removeComponent(ComponentType componentType) -> void {
        // m_components.erase(componentType);
    }

   private:
    EntityId m_id;
    std::unordered_map<ComponentType, std::shared_ptr<BaseComponent>> m_components;
};

}  // namespace Vengine