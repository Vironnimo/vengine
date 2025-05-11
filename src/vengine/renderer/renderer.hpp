#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/materials.hpp"
#include "vengine/renderer/window.hpp"
#include "vengine/renderer/shaders.hpp"
#include "vengine/renderer/fonts.hpp"
#include "vengine/renderer/skybox.hpp"
#include "vengine/core/scene.hpp"

namespace Vengine {

class Renderer {
   public:
    std::unique_ptr<Materials> materials;
    std::unique_ptr<Shaders> shaders;
    std::unique_ptr<Fonts> fonts;
    std::unique_ptr<Skybox> skybox;

    Renderer();
    ~Renderer();
    [[nodiscard]] auto init(std::shared_ptr<Window> window) -> tl::expected<void, Error>;

    auto render(const std::shared_ptr<Scene>& scene) -> void;
    auto setVSync(bool enabled) -> void;
    auto setMSAA(bool enabled) -> void;
    auto loadSkybox(const std::vector<std::shared_ptr<Texture>>& faceFiles) -> bool;
    auto unloadSkybox() -> void;

   private:
    std::shared_ptr<Window> m_window;
    bool m_skyboxEnabled = false;
};

}  // namespace Vengine