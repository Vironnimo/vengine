#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "vengine/core/resources.hpp"
#include "vengine/renderer/shader.hpp"

namespace Vengine {

class Material {
public:
    Material(std::shared_ptr<Shader> shader);
    ~Material() = default;

    // Shader management
    auto setShader(std::shared_ptr<Shader> shader) -> void;
    [[nodiscard]] auto getShader() const -> std::shared_ptr<Shader>;

    // Parameter setters
    auto setTexture(const std::string& name, std::shared_ptr<Texture> texture) -> void;
    auto setFloat(const std::string& name, float value) -> void;
    auto setInt(const std::string& name, int value) -> void;
    auto setVec2(const std::string& name, const glm::vec2& value) -> void;
    auto setVec3(const std::string& name, const glm::vec3& value) -> void;
    auto setVec4(const std::string& name, const glm::vec4& value) -> void;
    auto setMat4(const std::string& name, const glm::mat4& value) -> void;
    auto setBool(const std::string& name, bool value) -> void;

    // Bind this material (applies all parameters to shader)
    auto bind() const -> void;

    // Factory methods for common materials
    static auto createDefault() -> std::shared_ptr<Material>;
    static auto createColoredMaterial(const glm::vec4& color) -> std::shared_ptr<Material>;
    static auto createTexturedMaterial(std::shared_ptr<Texture> texture) -> std::shared_ptr<Material>;

private:
    std::shared_ptr<Shader> m_shader;
    
    // Uniform parameter storage
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, float> m_floats;
    std::unordered_map<std::string, int> m_ints;
    std::unordered_map<std::string, glm::vec2> m_vec2s;
    std::unordered_map<std::string, glm::vec3> m_vec3s;
    std::unordered_map<std::string, glm::vec4> m_vec4s;
    std::unordered_map<std::string, glm::mat4> m_mat4s;
    std::unordered_map<std::string, bool> m_bools;
};

} // namespace Vengine