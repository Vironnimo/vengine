#include "game.hpp"
#include <vconfig/vconfig.hpp>
#include "vengine/utils/utils.hpp"

namespace Vengine {

Game::Game() {
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
}

Game::~Game() {
    fmt::print("Game Destructor\n");
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
        m_renderer->render();
    }
}

}  // namespace Tetris