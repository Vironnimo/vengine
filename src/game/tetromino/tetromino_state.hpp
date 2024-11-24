#pragma once

#include <vector>

#include "game/global.hpp"

namespace Vengine {

enum TetrominoType { I, J, L, O, S, T, Z };

struct Block {
    Position position;
    Color color = {0.9f, 0.2f, 0.5f, 1.0f};
    bool isActive = false;

    void moveDown() {
        position.y -= 1;
    }

    void moveLeft() {
        position.x -= 1;
    }

    void moveRight() {
        position.x += 1;
    }
};

struct TetrominoState {
    // todo should each block also be made into a unique_ptr?
    std::vector<Block> blocks;  // positions of the blocks relative to the rotation point

    Position rotationPoint;
    Position position;  // position of the tetrono in the gamefield, also points to the rotation point
    TetrominoType type;
};

}  // namespace Tetris