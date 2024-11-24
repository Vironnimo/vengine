#pragma once

#include <vector>

#include "../component_manager.hpp"
#include "../components.hpp"

namespace Vengine {

using Entity = std::uint32_t;

class MovementSystem {
   public:
    void update(float deltaTime, const std::vector<Entity>& entities, ComponentManager<Position>& positionManager, ComponentManager<Velocity>& velocityManager) {
        // for each entity with position and velocity components
        // update the position based on the velocity
        for (Entity entity : entities)
        {
           if (positionManager.has(entity) && velocityManager.has(entity)) {
                auto& pos = positionManager.get(entity);
                auto& velocity = velocityManager.get(entity);
                pos.x += velocity.x * deltaTime;
                pos.y += velocity.y * deltaTime;
           } 
        }
    }
};

}