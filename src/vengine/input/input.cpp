#include "input.hpp"

namespace Vengine {

Input::Input(GLFWwindow* window) {
    fmt::print("Input constructor\n");

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
}

Input::~Input() {
}

bool Input::isKeyPressed(Key key) {
    return m_keys[key];
}

// todo make a list to save all the keys pressed and give that to the game
// todo create a map to map the keys to a game action
void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_UP:
                input->m_keys[Key::UP] = true;
                break;
            case GLFW_KEY_DOWN:
                input->m_keys[Key::DOWN] = true;
                break;
            case GLFW_KEY_LEFT:
                input->m_keys[Key::LEFT] = true;
                break;
            case GLFW_KEY_RIGHT:
                input->m_keys[Key::RIGHT] = true;
                break;
            case GLFW_KEY_ESCAPE:
                input->m_keys[Key::ESC] = true;
                break;
            case GLFW_KEY_SPACE:
                input->m_keys[Key::SPACE] = true;
                break;
            case GLFW_KEY_W:
                input->m_keys[Key::W] = true;
                break;
            case GLFW_KEY_A:
                input->m_keys[Key::A] = true;
                break;
            case GLFW_KEY_S:    
                input->m_keys[Key::S] = true;
                break;
            case GLFW_KEY_D:
                input->m_keys[Key::D] = true;
                break;
        }
    } else if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_UP:
                input->m_keys[Key::UP] = false;
                break;
            case GLFW_KEY_DOWN:
                input->m_keys[Key::DOWN] = false;
                break;
            case GLFW_KEY_LEFT:
                input->m_keys[Key::LEFT] = false;
                break;
            case GLFW_KEY_RIGHT:
                input->m_keys[Key::RIGHT] = false;
                break;
            case GLFW_KEY_ESCAPE:
                input->m_keys[Key::ESC] = false;
                break;
            case GLFW_KEY_SPACE:
                input->m_keys[Key::SPACE] = false;
                break;
            case GLFW_KEY_W:
                input->m_keys[Key::W] = false;
                break;
            case GLFW_KEY_A:
                input->m_keys[Key::A] = false;
                break;
            case GLFW_KEY_S:
                input->m_keys[Key::S] = false;
                break;
            case GLFW_KEY_D:
                input->m_keys[Key::D] = false;
                break;
        }
    }
}

}  // namespace Tetris