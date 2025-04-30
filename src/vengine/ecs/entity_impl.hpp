#pragma once

#include "vengine/ecs/entity.hpp"
#include "vengine/ecs/entities.hpp" 

namespace Vengine {

// this file is to avoid circular dependencies with entity and etitites

template <typename T, typename... Args>
auto Entity::addComponent(Args&&... args) -> void {
    if (isValid()) {
        m_manager->addComponent<T>(m_id, std::forward<Args>(args)...);
    }
}

template <typename T>
auto Entity::getComponent() -> std::shared_ptr<T> {
    if (isValid()) {
        return m_manager->getEntityComponent<T>(m_id);
    }
    return nullptr;
}

template <typename T>
auto Entity::hasComponent() -> bool {
    if (isValid()) {
        return m_manager->hasComponent<T>(m_id);
    }
    return false;
}

template <typename T>
auto Entity::removeComponent() -> void {
     if (isValid()) {
         m_manager->removeComponent<T>(m_id);
     }
}

} // namespace Vengine