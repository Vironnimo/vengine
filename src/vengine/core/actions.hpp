#pragma once

#include <functional>
#include <memory>
#include <string>
#include <GLFW/glfw3.h>
#include <unordered_map>

#include "action.hpp"

namespace Vengine {

class Actions {
   public:
    Actions();
    ~Actions();

    auto execute(const std::string& id) -> void;
    auto add(const std::string& id, std::function<void()> callback) -> bool;
    auto addKeybinding(const std::string& actionId, KeyBinding keyBind) -> void;

    auto isKeyBindingPressed(GLFWwindow* window, const KeyBinding& keyBind) -> bool;
    auto handleInput(GLFWwindow* window) -> void;

   private:
    std::unordered_map<std::string, std::unique_ptr<Action>> m_actions;
};

}  // namespace Vapp