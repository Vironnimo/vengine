#pragma once

#include <unordered_map>

namespace Vengine {

using Entity = std::uint32_t;

template <class Component>
class ComponentManager {
   public:
    void add(Entity entity, Component component);
    void remove(Entity entity);
    bool has(Entity entity);
    Component& get(Entity entity);
    
   private:
    std::unordered_map<Entity, Component> m_components;
};

template<class Component>
void ComponentManager<Component>::add(Entity entity, Component component) {
    m_components[entity] = component;
}

template<class Component>
void ComponentManager<Component>::remove(Entity entity) {
    m_components.erase(entity);
}

template<class Component>
bool ComponentManager<Component>::has(Entity entity) {
    return m_components.find(entity) != m_components.end();
}

template<class Component>
Component& ComponentManager<Component>::get(Entity entity) {
    return m_components.at(entity);
}

}  // namespace Engine