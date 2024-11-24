#include "ecs.hpp"

namespace Vengine {

ECS::ECS() {
    init();
}
void ECS::init() {
    // m_registry = std::make_unique<ComponentRegistry>();
}

Entity ECS::createEntity() {
    Entity entity = UUID::create();
    ComponentBitset bitset;
    m_entityComponents[entity] = bitset;
    return entity;
}

void ECS::destroyEntity(Entity entity) {
    m_entityComponents.erase(entity);
}

} // vEngine