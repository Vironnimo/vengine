#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>

namespace Vengine {

class InputManager {
public:
    InputManager();
    ~InputManager();

    void update(); 

    [[nodiscard]] auto isKeyDown(int key) const -> bool;
    [[nodiscard]] auto isKeyPressed(int key) const -> bool;
    [[nodiscard]] auto isKeyReleased(int key) const -> bool;

    [[nodiscard]] auto isMouseButtonDown(int button) const -> bool;
    [[nodiscard]] auto isMouseButtonPressed(int button) const -> bool;
    [[nodiscard]] auto isMouseButtonReleased(int button) const -> bool;

    [[nodiscard]] auto getMouseX() const -> double;
    [[nodiscard]] auto getMouseY() const -> double;
    [[nodiscard]] auto getMouseDeltaX() const -> double { return m_mouseDeltaX; }
    [[nodiscard]] auto getMouseDeltaY() const -> double { return m_mouseDeltaY; }

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