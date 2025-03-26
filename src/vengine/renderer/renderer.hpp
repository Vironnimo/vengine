#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/mesh.hpp"
#include "vengine/renderer/window.hpp"
#include "vengine/renderer/shaders.hpp"

namespace Vengine {

class Renderer {
   public:
    [[nodiscard]] auto init(std::shared_ptr<Window> window) -> tl::expected<void, Error>;

    auto render() -> void;
    auto setVSync(bool enabled) -> void;

   private:
    std::shared_ptr<Window> m_window;
    std::unique_ptr<Shaders> m_shaders;
    std::shared_ptr<Mesh> m_mesh_temp;
    std::shared_ptr<Mesh> m_mesh_temp2;
};

}  // namespace Vengine