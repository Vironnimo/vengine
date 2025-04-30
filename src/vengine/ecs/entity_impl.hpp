#pragma once

#include "vengine/ecs/entity.hpp"
#include "vengine/ecs/entities.hpp" 

namespace Vengine {

// this file is to avoid circular dependencies with entity and etitites

template <typename T, typename... Args>
auto Entity::addComponent(ComponentType componentType, Args&&... args) -> void {
    if (isValid()) {
        m_manager->addComponent<T>(m_id, componentType, std::forward<Args>(args)...);
    }
}

template <typename T>
auto Entity::getComponent(ComponentType componentType) -> std::shared_ptr<T> {
    if (isValid()) {
        return m_manager->getEntityComponent<T>(m_id, componentType);
    }
    return nullptr;
}

} // namespace Vengine