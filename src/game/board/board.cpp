#include "board.hpp"

namespace Vengine {

Board::Board() {
    fmt::print("Board Constructor\n");
    m_state = std::make_shared<BoardState>();

    // board is 10x20 (x, y) blocks, but the array is 20x10 (y, x)
    // fill board with empty blocks
    for (int i = 0; i < 20; i++) {
        addEmptyRow();
    }
}

Board::~Board() {
    fmt::print("Board Destructor\n");
}

void Board::addEmptyRow() {
    std::vector<std::unique_ptr<Block>> row;
    for (int j = 0; j < 10; j++) {
        row.push_back(std::make_unique<Block>());
    }
    m_state->board.push_back(std::move(row));
}

void Board::removeRow(int row) {
    m_state->board.erase(m_state->board.begin() + row);
    addEmptyRow();
}

void Board::attachTetromino(std::unique_ptr<Tetromino> tetromino) {
    for (auto& block : tetromino->getState()->blocks) {
        block.isActive = true;
        m_state->board[block.position.y][block.position.x] = std::make_unique<Block>(block);

    }
}

std::vector<int> Board::getFullRows() {
    std::vector<int> fullRows;
    for (int i = 0; i < 20; i++) {
        bool full = true;
        for (int j = 0; j < 10; j++) {
            if (!m_state->board[i][j]->isActive) {
                full = false;
                break;
            }
        }
        if (full) {
            fullRows.push_back(i);
        }
    }
    return fullRows;
}

std::shared_ptr<BoardState> Board::getState() {
    return m_state;
}

}  // namespace Tetris