#include "material.hpp"

#include <spdlog/spdlog.h>
#include <glad/glad.h>

#include <utility>

namespace Vengine {

Material::Material(std::shared_ptr<Shader> shader) : m_shader(std::move(shader)) {
    spdlog::debug("Constructor Material");
}

auto Material::setShader(std::shared_ptr<Shader> shader) -> void {
    m_shader = std::move(shader);
}

[[nodiscard]] auto Material::getShader() const -> std::shared_ptr<Shader> {
    return m_shader;
}

auto Material::setTexture(const std::string& name, std::shared_ptr<Texture> texture) -> void {
    m_textures[name] = std::move(texture);
}

auto Material::setFloat(const std::string& name, float value) -> void {
    m_floats[name] = value;
}

auto Material::setInt(const std::string& name, int value) -> void {
    m_ints[name] = value;
}

auto Material::setVec2(const std::string& name, const glm::vec2& value) -> void {
    m_vec2s[name] = value;
}

auto Material::setVec3(const std::string& name, const glm::vec3& value) -> void {
    m_vec3s[name] = value;
}

auto Material::setVec4(const std::string& name, const glm::vec4& value) -> void {
    m_vec4s[name] = value;
}

auto Material::setMat4(const std::string& name, const glm::mat4& value) -> void {
    m_mat4s[name] = value;
}

auto Material::setBool(const std::string& name, bool value) -> void {
    m_bools[name] = value;
}

auto Material::bind() const -> void {
    if (!m_shader) {
        spdlog::error("Cannot bind material: no shader assigned");
        return;
    }

    m_shader->bind();

    // texture
    int textureUnit = 0;
    for (const auto& [name, texture] : m_textures) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, texture->getTextureID());
        m_shader->setUniformInt(name, textureUnit);
        textureUnit++;
    }

    // apply uniforms
    for (const auto& [name, value] : m_floats) {
        m_shader->setUniformFloat(name, value);
    }

    for (const auto& [name, value] : m_ints) {
        m_shader->setUniformInt(name, value);
    }

    for (const auto& [name, value] : m_vec2s) {
        m_shader->setUniformVec2(name, value);
    }

    for (const auto& [name, value] : m_vec3s) {
        m_shader->setUniformVec3(name, value);
    }

    for (const auto& [name, value] : m_vec4s) {
        m_shader->setUniformVec4(name, value);
    }

    for (const auto& [name, value] : m_mat4s) {
        m_shader->setUniformMat4(name, value);
    }

    for (const auto& [name, value] : m_bools) {
        m_shader->setUniformBool(name, value);
    }
}

} // namespace Vengine