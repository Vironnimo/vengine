#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/shader.hpp"

namespace Vengine {

class Shaders {
   public:
    Shaders();
    ~Shaders();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;

    auto add(std::shared_ptr<Shader> shader) -> void;
    [[nodiscard]] auto get(const std::string& name) -> tl::expected<std::shared_ptr<Shader>, Error>;
    [[nodiscard]] auto exists(const std::string& name) const -> bool;
    auto remove(const std::string& name) -> void;
    auto clear() -> void;

   private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
};

}  // namespace Vengine