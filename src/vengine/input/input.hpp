#pragma once

#include <map>
#include <GLFW/glfw3.h>
#include <fmt/core.h>

namespace Vengine {

class Input {
   public:
    enum class Key { UP, DOWN, LEFT, RIGHT, ESC, SPACE, W, A, S, D};

    Input(GLFWwindow* window);
    ~Input();

    bool isKeyPressed(Key key);

   private:
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    // member variables
    std::map<Key, bool> m_keys;
};
 
}  // namespace Tetris