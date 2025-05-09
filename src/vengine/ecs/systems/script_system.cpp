#include "script_system.hpp"

#include "vengine/core/events.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"
#include "vengine/core/event_system.hpp"

namespace Vengine {

ScriptSystem::ScriptSystem() {
    m_luaState = luaL_newstate();
    if (!m_luaState) {
        spdlog::error("Failed to create Lua state for ScriptSystem!");
        return;
    }
    // load standard lua libs
    luaL_openlibs(m_luaState);
}

ScriptSystem::~ScriptSystem() {
    if (m_luaState) {
        lua_close(m_luaState);
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

void ScriptSystem::registerBindings(Vengine* vengine) {
    sol::state_view lua(m_luaState);

    // expose components to lua
    // GLFW key constants
    lua["GLFW_KEY_A"] = GLFW_KEY_A;
    lua["GLFW_KEY_D"] = GLFW_KEY_D;
    lua["GLFW_KEY_S"] = GLFW_KEY_S;
    lua["GLFW_KEY_W"] = GLFW_KEY_W;
    lua["GLFW_KEY_Q"] = GLFW_KEY_Q;
    lua["GLFW_KEY_E"] = GLFW_KEY_E;
    lua["GLFW_KEY_F"] = GLFW_KEY_F;
    lua["GLFW_KEY_R"] = GLFW_KEY_R;
    lua["GLFW_KEY_UP"] = GLFW_KEY_UP;
    lua["GLFW_KEY_DOWN"] = GLFW_KEY_DOWN;
    lua["GLFW_KEY_LEFT"] = GLFW_KEY_LEFT;
    lua["GLFW_KEY_RIGHT"] = GLFW_KEY_RIGHT;
    lua["GLFW_KEY_SPACE"] = GLFW_KEY_SPACE;
    lua["GLFW_KEY_ESCAPE"] = GLFW_KEY_ESCAPE;
    lua["GLFW_KEY_ENTER"] = GLFW_KEY_ENTER;
    lua["GLFW_KEY_BACKSPACE"] = GLFW_KEY_BACKSPACE;
    lua["GLFW_KEY_DELETE"] = GLFW_KEY_DELETE;
    // mouse
    lua["GLFW_MOUSE_BUTTON_LEFT"] = GLFW_MOUSE_BUTTON_LEFT;
    lua["GLFW_MOUSE_BUTTON_RIGHT"] = GLFW_MOUSE_BUTTON_RIGHT;
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

    // Expose InputSystem methods to Lua
    lua.new_usertype<InputSystem>("InputSystem",
                                  "isKeyDown",
                                  &InputSystem::isKeyDown,
                                  "isKeyPressed",
                                  &InputSystem::isKeyPressed,
                                  "isKeyReleased",
                                  &InputSystem::isKeyReleased,
                                  "isMouseButtonDown",
                                  &InputSystem::isMouseButtonDown,
                                  "isMouseButtonPressed",
                                  &InputSystem::isMouseButtonPressed,
                                  "isMouseButtonReleased",
                                  &InputSystem::isMouseButtonReleased,
                                  "getMouseX",
                                  &InputSystem::getMouseX,
                                  "getMouseY",
                                  &InputSystem::getMouseY,
                                  "getMouseDeltaX",
                                  &InputSystem::getMouseDeltaX,
                                  "getMouseDeltaY",
                                  &InputSystem::getMouseDeltaY);

    // Expose the input system instance as 'input'
    lua["input"] = vengine->inputSystem.get();

    // expose functions to lua, usage in lua: get_transform_component(entityId)
    lua["get_transform_component"] = [vengine](EntityId entityId) -> std::shared_ptr<TransformComponent> {
        return vengine->ecs->getActiveEntities()->getEntityComponent<TransformComponent>(entityId);
    };
    lua["get_camera_component"] = [vengine]() -> std::shared_ptr<CameraComponent> {
        return vengine->ecs->getActiveEntities()->getEntityComponent<CameraComponent>(
            vengine->scenes->getCurrentScene()->getCameras()->getActive());
    };

    lua.set_function("subscribe_event", [](sol::function lua_callback) {
        // Example: subscribe to KeyPressedEvent
        g_eventSystem.subscribe<KeyPressedEvent>(
            [lua_callback](const KeyPressedEvent& event) { lua_callback(event.key, event.repeat); });
    });

    lua.set_function("subscribe_mouse_event", [](sol::function lua_callback) {
        // Example: subscribe to KeyPressedEvent
        g_eventSystem.subscribe<MouseMovedEvent>(
            [lua_callback](const MouseMovedEvent& event) { lua_callback(event.x, event.y, event.lastX, event.lastY); });
    });
}

}  // namespace Vengine