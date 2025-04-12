#include "shaders.hpp"

#include <spdlog/spdlog.h>
#include <cassert>
#include <tl/expected.hpp>
#include "vengine/core/error.hpp"
#include "vengine/renderer/shader.hpp"

namespace Vengine {

Shaders::Shaders()
{
    spdlog::debug("Constructor Shaders");
}

Shaders::~Shaders() {
    spdlog::debug("Destructor Shaders");
    clear();
}

auto Shaders::init() -> tl::expected<void, Error> {
    // load some default shaders
    // Shader shader1{};
    // add(std::make_shared<Shader>("shader1", "resources/shaders/vertex.glsl", "resources/shaders/fragment.glsl"));
    // return tl::unexpected(Error{"Error loading shaders"});
    return {};
}

auto Shaders::add(std::shared_ptr<Shader> shader) -> void {
    assert(!shader->getName().empty() && "Shader name is empty");

    m_shaders[shader->getName()] = std::move(shader);
}

[[nodiscard]] auto Shaders::get(const std::string& name) -> tl::expected<std::shared_ptr<Shader>, Error> {
    assert(!name.empty() && "Shader name is empty");
    if (!exists(name)) {
        return tl::unexpected(Error{"Couldn't find Shader: " + name});
    }

    return m_shaders.at(name);
}

[[nodiscard]] auto Shaders::exists(const std::string& name) const -> bool {
    assert(!name.empty() && "Shader name is empty");
    auto result = m_shaders.find(name);
    return result != m_shaders.end();
}

auto Shaders::remove(const std::string& name) -> void {
    // todo still have to clear the shader itself, right?
    assert(!name.empty() && "Shader name is empty");
    m_shaders.erase(name);
}

auto Shaders::clear() -> void {
    m_shaders.clear();
}

}  // namespace Vengine