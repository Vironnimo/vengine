#pragma once


#include <sol/sol.hpp>

#include "vengine/ecs/ecs.hpp"
#include "vengine/ecs/base_system.hpp"

namespace Vengine {

class ScriptSystem : public BaseSystem {
   public:
    ScriptSystem();
    ~ScriptSystem() override; 

    void registerBindings(const std::shared_ptr<ECS>& ecs);
    void update(std::shared_ptr<Entities> entities, float deltaTime) override; 

   private:
    lua_State* m_luaState = nullptr;
};

}  // namespace Vengine