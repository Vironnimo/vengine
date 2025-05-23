#include "vengine/ecs/entity.hpp"
#include "vengine/ecs/entities.hpp" 

namespace Vengine {

Entity::Entity(EntityId id, Entities* manager) : m_id(id), m_manager(manager) {}
Entity::Entity() : m_id(0), m_manager(nullptr) {} 

auto Entity::getId() const -> EntityId {
    return m_id;
}

auto Entity::isValid() const -> bool {
    return m_manager != nullptr && m_id != 0;
}

auto Entity::destroy() -> void {
    if (isValid()) {
        m_manager->destroyEntity(m_id);
        m_id = 0;
        m_manager = nullptr;
    }
}

} // namespace Vengine