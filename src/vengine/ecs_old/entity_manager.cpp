#include "entity_manager.hpp"

namespace Vengine {

EntityManager::EntityManager() {}

EntityManager::~EntityManager() {
}

Entity EntityManager::createEntity() {
    Entity entity = m_nextEntityId++;
    m_entities.push_back(entity);
    return entity;
}

void EntityManager::destroyEntity(Entity entity) {
    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
}

std::vector<Entity>& EntityManager::getAllEntities() {
    return m_entities;
}

}  // namespace Engine