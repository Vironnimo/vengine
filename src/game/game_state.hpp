#pragma once

#include "board/board_state.hpp"
#include "tetromino/tetromino_state.hpp"

namespace Vengine {

enum class GameMode { GAME_RUNNING, MENU, PAUSE, GAME_OVER, EXIT };

class GameState {
   public:
    std::shared_ptr<BoardState> boardState;
    std::shared_ptr<TetrominoState> tetrominoState;

    GameMode mode = GameMode::GAME_RUNNING;

    static GameState& getInstance() {
        static GameState instance;
        return instance;
    }

    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;

   private:
    GameState() {
        fmt::print("GameState Constructor\n");
    }
};

}  // namespace Tetris
