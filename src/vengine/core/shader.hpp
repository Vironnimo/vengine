#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include "vengine/core/i_resource.hpp"

namespace Vengine {

class Shader : public IResource {
   public:
    Shader() = default;
    Shader(std::string name, const std::string& vertexFile, const std::string& fragmentFile);
    ~Shader() override;
    auto bind() const -> void;
    auto unbind() -> void;
    [[nodiscard]] auto getId() const -> GLuint;

    [[nodiscard]] auto load(const std::string& fileName) -> bool override;
    [[nodiscard]] auto unload() -> bool override;
            // move the stuff to mainthread!!
    [[nodiscard]] auto needsMainThreadInit() const -> bool override { return false; }
    [[nodiscard]] auto finalizeOnMainThread() -> bool override { return true; }

    // TODO uniform methods... which do we need, which could we need?
    auto setUniform1i(const std::string& name, int value) const -> void;
    auto setUniformMat4(const std::string& name, const glm::mat4& value) const -> void;
    auto setUniformVec4(const std::string& name, const glm::vec4& value) const -> void;
    auto setUniformInt(const std::string& name, int value) const -> void;
    auto setUniformBool(const std::string& name, bool value) const -> void;
    auto setUniformFloat(const std::string& name, float value) const -> void;
    auto setUniformVec2(const std::string& name, const glm::vec2& value) const -> void;
    auto setUniformVec3(const std::string& name, const glm::vec3& value) const -> void;

    [[nodiscard]] auto getName() -> const std::string&;

   private:
    GLuint m_id = 0;
    std::string m_name;

    // functions
    [[nodiscard]] auto readFile(const std::string& path) -> tl::expected<std::string, Error>;
    [[nodiscard]] auto compileShader(GLuint type, const std::string& source) -> tl::expected<GLuint, Error>;
};

}  // namespace Vengine