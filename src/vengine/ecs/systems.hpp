#pragma once

#include <spdlog/spdlog.h>
#include "entities.hpp"

#include "components.hpp"
#include "base_system.hpp"
#include "systems/script_system.hpp"
#include "systems/physics_system.hpp"

namespace Vengine {

class TransformSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float /*deltaTime*/) override {
        auto list = entities->getEntitiesWith<TransformComponent>();
        for (auto entityId : list) {
            auto transform = entities->getEntityComponent<TransformComponent>(entityId);
            if (transform && transform->dirty) {
                transform->updateMatrix();
                transform->dirty = false;
            }
        }
    }
};

}  // namespace Vengine