#pragma once

#include <vector>
#include <memory>

#include "game/global.hpp"
#include "tetromino_state.hpp"

namespace Vengine {

class Tetromino {
   public:
    Tetromino(TetrominoType type, Position position);
    ~Tetromino();

    // do we leave these direct implementations here?
    Position getPosition() const {
        return m_state->position;
    }
    TetrominoType getType() const {
        return m_state->type;
    }
    std::shared_ptr<TetrominoState> getState() {
        return m_state;
    }

    void setColor(Color color);
    void getTetrominoData(TetrominoType type);  // hopefully just temporary
    void calculateGamefieldPositions();  

    void rotateClockwise();
    void rotateCounterClockwise();
    void moveLeft();
    void moveRight();
    void moveDown();

   private:
    std::shared_ptr<TetrominoState> m_state;
};

}  // namespace Tetris