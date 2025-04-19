#pragma once

#include <memory>
#include "entities.hpp"

namespace Vengine {

class ECS;

class BaseSystem {
   public:
    virtual ~BaseSystem() = default;
    virtual void update(std::shared_ptr<Entities> entities, float deltaTime) = 0;
};

}  // namespace Vengine