#pragma once

#include "vengine/timer/timer.hpp"
#include "vengine/renderer/opengl_manager.hpp"
#include "vengine/input/input.hpp"
#include "vengine/renderer/renderer.hpp"

namespace Vengine {

class Game {
   public:
    Game();
    ~Game();

    void init();
    void mainLoop();

   private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<OpenGLManager> m_openGLManager;
};

}  // namespace Tetris