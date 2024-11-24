#pragma once

// #include <fmt/core.h>
#include "game/global.hpp"
#include "vengine/timer/timer.hpp"
#include "board/board.hpp"
#include "vengine/renderer/opengl_manager.hpp"
#include "vengine/input/input.hpp"
#include "vengine/renderer/renderer.hpp"
#include "game_state.hpp"

// test stuff
#include "game/tetromino/tetromino.hpp"

namespace Vengine {

class Game {
   public:
    Game();
    ~Game();

    void init();
    void mainLoop();

    void createNewTetromino();
    int createRandomNumber(int min, int max);
    float createRandomFloat(float max);

   private:
    GameState& m_gameState;

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Board> m_board;
    std::unique_ptr<Tetromino> m_activeTetromino;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<OpenGLManager> m_openGLManager;
};

}  // namespace Tetris