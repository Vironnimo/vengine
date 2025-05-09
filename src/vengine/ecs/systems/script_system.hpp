#pragma once


#include <sol/sol.hpp>

#include "vengine/ecs/base_system.hpp"

namespace Vengine {

class Vengine;

class ScriptSystem : public BaseSystem {
   public:
    ScriptSystem();
    ~ScriptSystem() override; 

    void registerBindings(Vengine* vengine);
    void update(std::shared_ptr<Entities> entities, float deltaTime) override; 

   private:
    lua_State* m_luaState = nullptr;
};

}  // namespace Vengine