#pragma once

#include <vector>
#include <memory>
#include "game/tetromino/tetromino_state.hpp"

namespace Vengine {

struct BoardState {
    std::vector<std::vector<std::unique_ptr<Block>>> board;
};

}  // namespace Tetris