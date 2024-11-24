#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <vector>

// cpp core guidelines stuff
#include <gsl/assert>

#include "game/game_state.hpp"
// #include "opengl/renderer/shaders/shader_manager.hpp"
#include "./shader_manager.hpp"

// temp
#include "game/tetromino/tetromino.hpp"

namespace Vengine {

class Renderer {
   public:
    Renderer();
    ~Renderer();

    void setWindow(GLFWwindow* window);

    void render(const GameState& state);

    void drawGrid();
    void drawGameFieldBorder();
    void drawRectangle(const glm::vec3& position, const Color* color);

    std::vector<Position> calculateGamefieldPositions(const GameState& state) const;    
    glm::vec3 convertPosition(std::pair<int, int> position);

   private:
    GLFWwindow* m_window;
    int m_offsetX = 6; // todo calculate this. 
    int m_offsetY = 1; // todo calculate this
    unsigned int m_blockVAO, m_blockVBO, m_blockEBO;
    unsigned int m_gridVAO, m_gridVBO;
    std::unique_ptr<ShaderManager> m_shaderManager;

    void initializeVertexData();
};

}  // namespace Tetris