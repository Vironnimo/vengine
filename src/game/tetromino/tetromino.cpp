#include "tetromino.hpp"

namespace Vengine {

Tetromino::Tetromino(TetrominoType type, Position position) {
    fmt::print("Tetromino Constructor\n");

    // we get the tetromino data, as relative points and convert them straight to gamefield positions
    m_state = std::make_shared<TetrominoState>();
    getTetrominoData(type);
    m_state->position = position;
    calculateGamefieldPositions();
}

Tetromino::~Tetromino() {
}

void Tetromino::setColor(Color color) {
    for (auto& block : m_state->blocks) {
        block.color = color;
    }
}

void Tetromino::calculateGamefieldPositions() {
    for (auto& block : m_state->blocks) {
        block.position = m_state->position + block.position;
    }
}

// todo put this somewhere else, probably in some other form
void Tetromino::getTetrominoData(TetrominoType type) {
    switch (type) {
        case TetrominoType::I:
            m_state->blocks = {{0, 0}, {0, 1}, {0, 2}, {0, 3}};
            m_state->rotationPoint = {0, 1};
            break;
        case TetrominoType::J:
            m_state->blocks = {{0, 0}, {0, 1}, {0, 2}, {1, 2}};
            m_state->rotationPoint = {0, 1};
            break;
        case TetrominoType::L:
            m_state->blocks = {{0, 0}, {0, 1}, {0, 2}, {-1, 2}};
            m_state->rotationPoint = {0, 1};
            break;
        case TetrominoType::O:
            m_state->blocks = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
            m_state->rotationPoint = {0, 0};
            break;
        case TetrominoType::S:
            m_state->blocks = {{0, 0}, {0, 1}, {1, 1}, {1, 2}};
            m_state->rotationPoint = {0, 1};
            break;
        case TetrominoType::T:
            m_state->blocks = {{0, 0}, {0, 1}, {0, 2}, {1, 1}};
            m_state->rotationPoint = {0, 1};
            break;
        case TetrominoType::Z:
            m_state->blocks = {{0, 0}, {0, 1}, {-1, 1}, {-1, 2}};
            m_state->rotationPoint = {0, 1};
            break;
    }
}

void Tetromino::rotateClockwise() {
    int centerY = m_state->blocks[1].position.x;
    int centerX = m_state->blocks[1].position.y;

    for (auto& block : m_state->blocks) {
        int tmpX = centerY + block.position.y - centerX;
        int tmpY = centerX - block.position.x + centerY;
        block.position.x = tmpX;
        block.position.y = tmpY;
    }

    // todo should this be here? at least the hardcoded numbers should be somewhere else
    // and we can combine it with the for loop above
    // debounce on walls
    for (auto& block : m_state->blocks) {
        if (block.position.x < 0) {
            moveRight();
        }
        if (block.position.x > 9) {
            moveLeft();
        }
    }
}

void Tetromino::rotateCounterClockwise() {
    // todo copy from above 
}

void Tetromino::moveLeft() {
    for (auto& block : m_state->blocks) {
        block.moveLeft();
    }
}

void Tetromino::moveRight() {
    for (auto& block : m_state->blocks) {
        block.moveRight();
    }
}

void Tetromino::moveDown() {
    for (auto& block : m_state->blocks) {
        block.moveDown();
    }
}

}  // namespace Tetris