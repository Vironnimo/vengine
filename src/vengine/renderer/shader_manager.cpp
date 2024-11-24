#include "shader_manager.hpp"

namespace Vengine {

ShaderManager::ShaderManager() {
    fmt::print("ShaderManager Constructor\n");

    loadShaderSourcesFromFile("resources/shaders/shader.vert.glsl", "resources/shaders/shader.frag.glsl");
    compileAndLinkShaders();
}

void ShaderManager::loadShaderSourcesFromFile(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    std::ifstream vertexShaderFile;
    std::ifstream fragmentShaderFile;

    vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vertexShaderFile.open(vertexShaderPath);
        fragmentShaderFile.open(fragmentShaderPath);

        std::stringstream vertexShaderStream, fragmentShaderStream;

        vertexShaderStream << vertexShaderFile.rdbuf();
        fragmentShaderStream << fragmentShaderFile.rdbuf();

        m_vertexShaderSource = vertexShaderStream.str();
        m_fragmentShaderSource = fragmentShaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    vertexShaderFile.close();
    fragmentShaderFile.close();
}

void ShaderManager::transform(glm::mat4 modelMatrix, const Color* color) {
    // glm::mat4 trans = glm::mat4(1.0f);
    // trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
    // trans = glm::scale(trans, glm::vec3(1.0, 1.0, 1.0));
    // trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.0f));

    GLint transformLoc = glGetUniformLocation(m_shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLint colorLoc = glGetUniformLocation(m_shaderProgram, "uColor");
    glUniform4f(colorLoc, color->r, color->g, color->b, color->a);
}

void ShaderManager::compileAndLinkShaders() {
    const char* vertexShaderSource = m_vertexShaderSource.c_str();
    const char* fragmentShaderSource = m_fragmentShaderSource.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
 
GLuint ShaderManager::getShaderProgram() {
    return m_shaderProgram;
}

}  // namespace Engine