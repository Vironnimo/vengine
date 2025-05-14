#include "shader.hpp"

#include <utility>
#include <sstream>
#include <fstream>

#include <cassert>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"

namespace Vengine {

Shader::Shader(std::string name, const std::string& vertexFile, const std::string& fragmentFile)
    : m_name(std::move(name)) {
    spdlog::debug("Constructor Shader: {}", m_name);
    // TODO put this into a init() function so we can cut execution on error, right?
    assert(!m_name.empty() && "Shader name is empty");
    assert(!vertexFile.empty() && "Vertex shader file is empty");
    assert(!fragmentFile.empty() && "Fragment shader file is empty");

    // compile and link shaders, i guess
    auto vertexSource = readFile(vertexFile);
    if (!vertexSource) {
        spdlog::error("{}", vertexSource.error().message);
    }
    auto fragmentSource = readFile(fragmentFile);
    if (!fragmentSource) {
        spdlog::error("{}", fragmentSource.error().message);
    }

    auto vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.value());
    if (!vertexShader) {
        spdlog::error("{}", vertexShader.error().message);
    }

    auto fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.value());
    if (!fragmentShader) {
        spdlog::error("{}", fragmentShader.error().message);
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShader.value());
    glAttachShader(m_id, fragmentShader.value());
    glLinkProgram(m_id);

    int success;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (success == 0) {
        std::string infoLog;
        infoLog.resize(512);  // Pre-allocate space
        glGetProgramInfoLog(m_id, 512, nullptr, infoLog.data());
        spdlog::error("{}", "Shader program linking failed: " + infoLog);
    }

    glDeleteShader(vertexShader.value());
    glDeleteShader(fragmentShader.value());
}

Shader::~Shader() {
    spdlog::debug("Destructor Shader: {}", m_name);
    glDeleteProgram(m_id);
    m_id = 0;
    m_name.clear();
}

auto Shader::bind() const -> void {
    glUseProgram(m_id);
}

auto Shader::unbind() -> void {
    glUseProgram(0);
}

[[nodiscard]] auto Shader::compileShader(GLuint type, const std::string& source) -> tl::expected<GLuint, Error> {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        std::string infoLog;
        infoLog.resize(512);
        glGetShaderInfoLog(shader, 512, nullptr, infoLog.data());
        return tl::unexpected(Error{"Shader compilation failed: " + infoLog});
    }

    return shader;
}

[[nodiscard]] auto Shader::readFile(const std::string& path) -> tl::expected<std::string, Error> {
    std::ifstream file(path);
    if (!file) {
        return tl::unexpected(Error{"Could not open file: " + path});
        // throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

[[nodiscard]] auto Shader::getName() -> const std::string& {
    return m_name;
}

[[nodiscard]] auto Shader::getId() const -> GLuint {
    return m_id;
}

auto Shader::setUniform1i(const std::string& name, int value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(location, value);

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

auto Shader::setUniformMat4(const std::string& name, const glm::mat4& value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    // &value[0][0] gives us the memory address of the first element (top-left) in a mat4,
    // which creates a pointer to the first element
    glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

auto Shader::setUniformVec4(const std::string& name, const glm::vec4& value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    // &value[0] gives us the memory address of the first element in a vec4,
    // which creates a pointer to the first element
    glUniform4fv(location, 1, &value[0]);

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

auto Shader::setUniformInt(const std::string& name, int value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(location, value);

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

auto Shader::setUniformBool(const std::string& name, bool value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(location, static_cast<int>(value));

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

auto Shader::setUniformFloat(const std::string& name, float value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1f(location, value);

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

auto Shader::setUniformVec2(const std::string& name, const glm::vec2& value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform2fv(location, 1, &value[0]);

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

auto Shader::setUniformVec3(const std::string& name, const glm::vec3& value) const -> void {
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform3fv(location, 1, &value[0]);

    if (location == -1) {
        spdlog::error("Uniform '{}' not found in shader program", name);
    }
}

}  // namespace Vengine
