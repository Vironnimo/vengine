#include "actions.hpp"

#include <spdlog/spdlog.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace Vengine {
Actions::Actions() {
    spdlog::debug("Constructor Actions");
}

Actions::~Actions() {
    spdlog::debug("Destructor Actions");
}

auto Actions::execute(const std::string& id) -> void {
    auto it = m_actions.find(id);
    if (it != m_actions.end()) {
        it->second->execute();
    }
}

auto Actions::add(const std::string& id, const std::string& name, std::function<void()> callback) -> bool {
    auto it = m_actions.find(id);
    if (it != m_actions.end()) {
        return false;
    }

    m_actions.emplace(id, std::make_unique<Action>(id, name, std::move(callback)));
    return true;
}

auto Actions::addKeybinding(const std::string& actionId, KeyBinding keyBind) -> void {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        it->second->addKeyBinding(keyBind);
    }
}

auto Actions::isKeyBindingPressed(GLFWwindow* window, const KeyBinding& keyBind) -> bool {
    if (keyBind.key == GLFW_MOUSE_BUTTON_LEFT || keyBind.key == GLFW_MOUSE_BUTTON_RIGHT) {
        return glfwGetMouseButton(window, keyBind.key) == GLFW_PRESS;
    }

    const bool keyPressed = glfwGetKey(window, keyBind.key) == GLFW_PRESS;
    const bool ctrlHeld = keyBind.ctrl == (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                                           glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
    const bool shiftHeld = keyBind.shift == (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                             glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    const bool altHeld = keyBind.alt == (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
                                         glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);

    return keyPressed && ctrlHeld && shiftHeld && altHeld;
}

auto Actions::handleInput(GLFWwindow* window) -> void {
    for (const auto& [id, action] : m_actions) {
        for (const auto& binding : action->getKeyBindings()) {
            if (isKeyBindingPressed(window, binding)) {
                action->execute();
                break;
            }
        }
    }
}

}  // namespace Vengine