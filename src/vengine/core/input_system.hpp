#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>

namespace Vengine {

class InputSystem {
public:
    void update(); 

    auto isKeyDown(int key) const -> bool;
    auto isKeyPressed(int key) const -> bool;
    auto isKeyReleased(int key) const -> bool;

    auto isMouseButtonDown(int button) const -> bool;
    auto isMouseButtonPressed(int button) const -> bool;
    auto isMouseButtonReleased(int button) const -> bool;

    auto getMouseX() const -> double;
    auto getMouseY() const -> double;
    auto getMouseDeltaX() const -> double { return m_mouseDeltaX; }
    auto getMouseDeltaY() const -> double { return m_mouseDeltaY; }

    void setWindow(GLFWwindow* window);

private:
    GLFWwindow* m_window = nullptr;
    std::unordered_map<int, bool> m_keysDown;
    std::unordered_map<int, bool> m_keysPressed;
    std::unordered_map<int, bool> m_keysReleased;

    std::unordered_map<int, bool> m_mouseDown;
    std::unordered_map<int, bool> m_mousePressed;
    std::unordered_map<int, bool> m_mouseReleased;

    double m_mouseX = 0.0, m_mouseY = 0.0;
    double m_lastMouseX = 0.0, m_lastMouseY = 0.0;
    double m_mouseDeltaX = 0.0, m_mouseDeltaY = 0.0;

    void pollInput();
};

} // namespace Vengine