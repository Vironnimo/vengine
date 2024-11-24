#include "game.hpp"

namespace Vengine {

Game::Game() : m_gameState(GameState::getInstance()) {
    fmt::print("Game Constructor\n");
    init();
}

void Game::init() {
    fmt::print("Game init\n");

    m_openGLManager = std::make_unique<OpenGLManager>();
    m_openGLManager->initialize();
    m_openGLManager->createWindow("Renderer", 800, 800);

    m_renderer = std::make_unique<Renderer>();
    m_renderer->setWindow(m_openGLManager->getWindow());

    m_input = std::make_unique<Input>(m_openGLManager->getWindow());

    // init game state
    m_board = std::make_unique<Board>();
    m_gameState.boardState = m_board->getState();
    createNewTetromino();
}

Game::~Game() {
    fmt::print("Game Destructor\n");
}

void Game::createNewTetromino() {
    // get random tetromino type
    int random = createRandomNumber(0, 6);
    auto type = static_cast<TetrominoType>(random);

    m_activeTetromino = std::make_unique<Tetromino>(type, Position{5, 16});
    // set random color
    Color rndColor = {createRandomFloat(1.0f), createRandomFloat(1.0f), createRandomFloat(1.0f), 1.0f};
    m_activeTetromino->setColor(rndColor);
    m_gameState.tetrominoState = m_activeTetromino->getState();
}

int Game::createRandomNumber(int min, int max) {
    return min + (rand() % static_cast<int>(max - min + 1));
}

float Game::createRandomFloat(float max) {
    // create random number between 0.0 and max
    return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / max));
}

void Game::mainLoop() {
    fmt::print("Game running\n");

    while (!glfwWindowShouldClose(m_openGLManager->getWindow())) {
        // show fps
        // fmt::print("FPS: {}\n", 1.0f / Timer::getDeltaTime());

        // get input
        if (m_input->isKeyPressed(Input::Key::ESC)) {
            fmt::print("ESC pressed\n");
            glfwSetWindowShouldClose(m_openGLManager->getWindow(), true);
        }
        if (m_input->isKeyPressed(Input::Key::LEFT)) {
            fmt::print("Move left\n");
        }
        if (m_input->isKeyPressed(Input::Key::RIGHT)) {
            fmt::print("Move right\n");
        }
        if (m_input->isKeyPressed(Input::Key::UP)) {
            fmt::print("Rotate\n");
        }
        if (m_input->isKeyPressed(Input::Key::DOWN)) {
            fmt::print("Move down\n");
        }

        // render
        m_renderer->render(m_gameState);

    }
}

}  // namespace Tetris