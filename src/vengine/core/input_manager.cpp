#include "input_manager.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "vengine/core/event_manager.hpp"

namespace Vengine {

InputManager::InputManager() {
    spdlog::debug("Constructor InputSystem");
}

InputManager::~InputManager() {
    spdlog::debug("Destructor InputSystem");
}

void InputManager::setWindow(GLFWwindow* window) {
    assert(window != nullptr && "Window cannot be null");
    m_window = window;
}

void InputManager::update() {
    m_keysPressed.clear();
    m_keysReleased.clear();
    m_mousePressed.clear();
    m_mouseReleased.clear();

    pollInput();
}

void InputManager::pollInput() {
    if (!m_window) {
        spdlog::warn("InputManager: No window set, cannot poll input");
        return;
    }

    // keyboard
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        int state = glfwGetKey(m_window, key);
        bool wasDown = m_keysDown[key];
        bool down = state == GLFW_PRESS || state == GLFW_REPEAT;

        // fresh press
        if (state == GLFW_PRESS && !wasDown) {
            m_keysPressed[key] = true;
        }

        // repeat
        if (state == GLFW_REPEAT) {
            spdlog::debug("repeat");
        }

        // release
        if (!down && wasDown) {
            m_keysReleased[key] = true;
        }
        m_keysDown[key] = down;
    }

    // mouse
    for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
        int state = glfwGetMouseButton(m_window, button);
        bool down = state == GLFW_PRESS;
        if (down && !m_mouseDown[button]) {
            m_mousePressed[button] = true;
        }
        if (!down && m_mouseDown[button]) {
            m_mouseReleased[button] = true;
        }
        m_mouseDown[button] = down;
    }

    m_lastMouseX = m_mouseX;
    m_lastMouseY = m_mouseY;
    glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);
    m_mouseDeltaX = m_mouseX - m_lastMouseX;
    m_mouseDeltaY = m_mouseY - m_lastMouseY;

    // send mouse moved event
    if (m_mouseDeltaX != 0.0 || m_mouseDeltaY != 0.0) {
        g_eventManager.publish(MouseMovedEvent{static_cast<int>(m_mouseX),
                                              static_cast<int>(m_mouseY),
                                              static_cast<int>(m_lastMouseX),
                                              static_cast<int>(m_lastMouseY)});
    }
}

auto InputManager::isKeyDown(int key) const -> bool {
    auto it = m_keysDown.find(key);
    return it != m_keysDown.end() && it->second;
}
auto InputManager::isKeyPressed(int key) const -> bool {
    auto it = m_keysPressed.find(key);
    return it != m_keysPressed.end() && it->second;
}
auto InputManager::isKeyReleased(int key) const -> bool {
    auto it = m_keysReleased.find(key);
    return it != m_keysReleased.end() && it->second;
}

auto InputManager::isMouseButtonDown(int button) const -> bool {
    auto it = m_mouseDown.find(button);
    return it != m_mouseDown.end() && it->second;
}
auto InputManager::isMouseButtonPressed(int button) const -> bool {
    auto it = m_mousePressed.find(button);
    return it != m_mousePressed.end() && it->second;
}
auto InputManager::isMouseButtonReleased(int button) const -> bool {
    auto it = m_mouseReleased.find(button);
    return it != m_mouseReleased.end() && it->second;
}

auto InputManager::getMouseX() const -> double {
    return m_mouseX;
}
auto InputManager::getMouseY() const -> double {
    return m_mouseY;
}

}  // namespace Vengine