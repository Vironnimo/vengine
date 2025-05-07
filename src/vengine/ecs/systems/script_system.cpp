#include "script_system.hpp"

#include "vengine/ecs/components.hpp"

namespace Vengine {
ScriptSystem::ScriptSystem() {
    m_luaState = luaL_newstate();
    if (!m_luaState) {
        spdlog::error("Failed to create Lua state for ScriptSystem!");
        return;
    }
    // load standard lua libs
    luaL_openlibs(m_luaState);
    spdlog::debug("ScriptSystem initialized Lua state.");
}

ScriptSystem::~ScriptSystem() {
    if (m_luaState) {
        lua_close(m_luaState);
        spdlog::debug("ScriptSystem closed Lua state.");
    }
}

void ScriptSystem::update(std::shared_ptr<Entities> entities, float deltaTime) {
    if (!m_luaState) {
        spdlog::error("ScriptSystem has no valid Lua state.");
        return;
    }

    auto list = entities->getEntitiesWith<ScriptComponent>();
    for (auto entityId : list) {
        auto scriptComp = entities->getEntityComponent<ScriptComponent>(entityId);
        if (!scriptComp || scriptComp->path.empty()) {
            continue;
        }

        // load script file
        // TODO don't load it every frame, only when it changes (or just once?)
        if (luaL_dofile(m_luaState, scriptComp->path.c_str()) != LUA_OK) {
            spdlog::error("Error loading/running Lua script '{}': {}", scriptComp->path, lua_tostring(m_luaState, -1));
            lua_pop(m_luaState, 1);
            continue;
        }

        // call update function in script
        lua_getglobal(m_luaState, "update");
        if (lua_isfunction(m_luaState, -1)) {
            // arguments: entityId and deltaTime for now?
            lua_pushinteger(m_luaState, static_cast<lua_Integer>(entityId));
            lua_pushnumber(m_luaState, static_cast<lua_Number>(deltaTime));

            // Call the function: 2 arguments, 0 results
            if (lua_pcall(m_luaState, 2, 0, 0) != LUA_OK) {
                spdlog::error("Error calling Lua function 'update' in script '{}': {}",
                              scriptComp->path,
                              lua_tostring(m_luaState, -1));
                lua_pop(m_luaState, 1);
            }
        } else {
            // Pop the non-function value if it's not nil
            if (!lua_isnil(m_luaState, -1)) {
                spdlog::trace("Script '{}' does not have an 'update' function.", scriptComp->path);
                lua_pop(m_luaState, 1);
            }
        }
        // Clean up stack potentially left by dofile if it returned values
        lua_settop(m_luaState, 0);
    }
}

void ScriptSystem::registerBindings(const std::shared_ptr<ECS>& ecs) {
    sol::state_view lua(m_luaState);

    // expose components to lua
    // TransformComponent
    sol::usertype<TransformComponent> transform_type = lua.new_usertype<TransformComponent>(
        "TransformComponent",
        sol::no_constructor,
        "getPositionX",
        &TransformComponent::getPositionX,
        "getPositionY",
        &TransformComponent::getPositionY,
        "getPositionZ",
        &TransformComponent::getPositionZ,
        "getRotationX",
        &TransformComponent::getRotationX,
        "getRotationY",
        &TransformComponent::getRotationY,
        "getRotationZ",
        &TransformComponent::getRotationZ,
        "getScaleX",
        &TransformComponent::getScaleX,
        "getScaleY",
        &TransformComponent::getScaleY,
        "getScaleZ",
        &TransformComponent::getScaleZ,
        "setPosition",
        sol::overload(static_cast<void (TransformComponent::*)(float)>(&TransformComponent::setPosition),
                      static_cast<void (TransformComponent::*)(float, float, float)>(&TransformComponent::setPosition)),
        "setRotation",
        sol::overload(static_cast<void (TransformComponent::*)(float)>(&TransformComponent::setRotation),
                      static_cast<void (TransformComponent::*)(float, float, float)>(&TransformComponent::setRotation)),
        "setScale",
        sol::overload(static_cast<void (TransformComponent::*)(float)>(&TransformComponent::setScale),
                      static_cast<void (TransformComponent::*)(float, float, float)>(&TransformComponent::setScale)),
        "updateMatrix",
        &TransformComponent::updateMatrix);

    // CameraComponent
    sol::usertype<CameraComponent> camera_type = lua.new_usertype<CameraComponent>("CameraComponent",
                                                                                   sol::no_constructor,
                                                                                   "getProjectionMatrix",
                                                                                   &CameraComponent::getProjectionMatrix,
                                                                                   "getFov",
                                                                                   &CameraComponent::getFov,
                                                                                   "getAspectRatio",
                                                                                   &CameraComponent::getAspectRatio,
                                                                                   "getNearPlane",
                                                                                   &CameraComponent::getNearPlane,
                                                                                   "getFarPlane",
                                                                                   &CameraComponent::getFarPlane,
                                                                                   "setFov",
                                                                                   &CameraComponent::setFov,
                                                                                   "setAspectRatio",
                                                                                   &CameraComponent::setAspectRatio,
                                                                                   "setNearPlane",
                                                                                   &CameraComponent::setNearPlane,
                                                                                   "setFarPlane",
                                                                                   &CameraComponent::setFarPlane,
                                                                                   "setActive",
                                                                                   &CameraComponent::setActive,
                                                                                   "isActiveCamera",
                                                                                   &CameraComponent::isActiveCamera);

    // expose functions to lua, usage in lua: get_transform_component(entityId)
    lua["get_transform_component"] = [ecs](EntityId entityId) -> std::shared_ptr<TransformComponent> {
        return ecs->getActiveEntities()->getEntityComponent<TransformComponent>(entityId);
    };
    lua["get_camera_component"] = [ecs]() -> std::shared_ptr<CameraComponent> {
        // TODO need access to the cameras for active camera
        // return ecs->getActiveEntities()->getEntityComponent<CameraComponent>(ecs->getActiveCamera());
    };
}

}  // namespace Vengine