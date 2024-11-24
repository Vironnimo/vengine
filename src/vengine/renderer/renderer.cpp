#include "renderer.hpp"

namespace Vengine {

Renderer::Renderer() {
    fmt::print("Renderer Constructor\n");

    m_shaderManager = std::make_unique<ShaderManager>();
    initializeVertexData();
}

Renderer::~Renderer() {
    fmt::print("Renderer Destructor\n");
}

void Renderer::initializeVertexData() {
    // grid

    // Define the grid dimensions
    const int rows = 22;
    const int cols = 22;
    const float rectWidth = 2.0f / rows;

    // Create a vector to hold the vertices for the grid lines
    std::vector<float> gridVertices;

    // Generate the vertices for the horizontal lines
    for (int row = 0; row <= rows; ++row) {
        float y = 1.0f - row * rectWidth;  // starting from top
        gridVertices.push_back(-1.0f);     // x1
        gridVertices.push_back(y);         // y1
        gridVertices.push_back(0.0f);      // z1
        gridVertices.push_back(1.0f);      // x2
        gridVertices.push_back(y);         // y2
        gridVertices.push_back(0.0f);      // z2
    }

    // Generate the vertices for the vertical lines
    for (int col = 0; col <= cols; ++col) {
        float x = -1.0f + col * rectWidth;  // starting from left
        gridVertices.push_back(x);          // x1
        gridVertices.push_back(1.0f);       // y1
        gridVertices.push_back(0.0f);       // z1
        gridVertices.push_back(x);          // x2
        gridVertices.push_back(-1.0f);      // y2
        gridVertices.push_back(0.0f);       // z2
    }

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);

    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // the height should be 22 blocks
    float blockSize = 2.0f / 22.0f;
    // starting position is bottom left
    float startingX = -1.0f;
    float startingY = -1.0f;

    float vertices[] = {
        startingX,  // bottom left
        startingY,
        0.0f,
        startingX,  // bottom right
        startingY + blockSize,
        0.0f,
        startingX + blockSize,  // top right
        startingY + blockSize,
        0.0f,
        startingX + blockSize,  // top left
        startingY,
        0.0f,
    };

    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    // block vao, vbo and ebo
    glGenVertexArrays(1, &m_blockVAO);
    glGenBuffers(1, &m_blockVBO);
    glGenBuffers(1, &m_blockEBO);
    glBindVertexArray(m_blockVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_blockVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_blockEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::drawGrid() {
    glUseProgram(m_shaderManager->getShaderProgram());
    glBindVertexArray(m_gridVAO);

    // Set the color for the grid lines
    Color color = {0.4f, 0.4f, 0.4f, 1.0f};
    glm::mat4 model = glm::mat4(1.0f);
    m_shaderManager->transform(model, &color);

    // Draw the grid lines
    glDrawArrays(GL_LINES, 0, 276);

    glBindVertexArray(0);
}

void Renderer::drawRectangle(const glm::vec3& position, const Color* color) {
    glUseProgram(m_shaderManager->getShaderProgram());
    glBindVertexArray(m_blockVAO);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);  // glm::vec3(position[0], position[1], position[2]));
    // model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    m_shaderManager->transform(model, color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::drawGameFieldBorder() {
    // draw the gamefield border
    Color borderBlockColor = {0.24f, 0.24f, 0.24f, 1.0f};
    for (int i = 0; i < 22; i++) {
        drawRectangle(convertPosition(Position{5, i}.toPair()), &borderBlockColor);
        drawRectangle(convertPosition(Position{16, i}.toPair()), &borderBlockColor);
    }

    for (int i = 6; i <= 16; i++) {
        drawRectangle(convertPosition(Position{i, 0}.toPair()), &borderBlockColor);
        drawRectangle(convertPosition(Position{i, 21}.toPair()), &borderBlockColor);
    }
}

// convert a position in the gamefield to a position in the screen
glm::vec3 Renderer::convertPosition(std::pair<int, int> position) {
    // assuming the screen is 22x22 blocks
    float x = (position.first * (2.0f / 22.0f));
    float y = (position.second * (2.0f / 22.0f));
    return glm::vec3{x, y, 0.0f};
}

void Renderer::render(const GameState& state) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw blocks on the board
    // todo need an offset here to draw the blocks in the correct position
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            if (state.boardState->board[i][j]->isActive) {
                std::pair<int, int> position = Position{j, i}.toPair();
                position.first += m_offsetX;
                position.second += m_offsetY;
                drawRectangle(convertPosition(position), &state.boardState->board[i][j]->color);
            }
        }
    }

    // draw the active tetromino
    // std::vector<Position> positions = calculateGamefieldPositions(state);
    // for (const auto& position : positions) {
    //     std::pair<int, int> tmpPosition = position.toPair();
    //     tmpPosition.first += m_offsetX;
    //     tmpPosition.second += m_offsetY;
    //     drawRectangle(convertPosition(tmpPosition), &state.tetrominoState->blocks[0].color);
    // }

    for (const auto& block : state.tetrominoState->blocks) {
        std::pair<int, int> tmpPosition = block.position.toPair();
        tmpPosition.first += m_offsetX;
        tmpPosition.second += m_offsetY;
        drawRectangle(convertPosition(tmpPosition), &block.color);
    }

    drawGameFieldBorder();
    drawGrid();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

// convert the positions of the tetromino block (which are relative to the rotationpoint) to positions on the gamefield
std::vector<Position> Renderer::calculateGamefieldPositions(const GameState& state) const {
    std::vector<Position> boardPositions;

    for (const auto& block : state.tetrominoState->blocks) {
        Position boardPosition =
            state.tetrominoState->position + (block.position - state.tetrominoState->rotationPoint);
        boardPositions.push_back(boardPosition);
    }

    return boardPositions;
}

void Renderer::setWindow(GLFWwindow* window) {
    m_window = window;
}

}  // namespace Tetris
