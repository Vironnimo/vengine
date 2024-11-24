#pragma once

#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>


namespace Vengine {

using Entity = std::uint32_t;

class EntityManager {
   public:
    EntityManager();
    ~EntityManager();

    Entity createEntity();
    void destroyEntity(Entity entity);
    std::vector<Entity>& getAllEntities();

   private:
    std::vector<Entity> m_entities;
    std::uint32_t m_nextEntityId = 0;
};

}  // namespace Engine