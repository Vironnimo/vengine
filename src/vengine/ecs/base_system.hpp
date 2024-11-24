#pragma once

namespace Vengine {

class ECS;

class BaseSystem {
   public:
    virtual ~BaseSystem() = default;
    virtual void update(ECS* ecs, float deltaTime) = 0;
};

}  // namespace Vengine