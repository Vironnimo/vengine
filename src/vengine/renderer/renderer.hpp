#pragma once

#include <cstddef>
#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/core/resources.hpp"
#include "vengine/renderer/materials.hpp"
#include "vengine/renderer/window.hpp"
// #include "vengine/core/shaders.hpp"
#include "vengine/renderer/fonts.hpp"
#include "vengine/renderer/skybox.hpp"
#include "vengine/core/scene.hpp"

namespace Vengine {

class Renderer {
   public:
    std::unique_ptr<Materials> materials;
    // std::unique_ptr<Shaders> shaders;
    std::unique_ptr<Fonts> fonts;
    std::unique_ptr<Skybox> skybox;

    Renderer();
    ~Renderer();
    [[nodiscard]] auto init(std::shared_ptr<Window> window) -> tl::expected<void, Error>;
    auto initFonts(std::shared_ptr<Shader> fontShader) -> tl::expected<void, Error>;
    auto setShadowShader(std::shared_ptr<Shader> shader) -> void;

    auto render(const std::shared_ptr<Scene>& scene) -> void;
    auto setVSync(bool enabled) -> void;
    [[nodiscard]] auto isVSyncEnabled() const -> bool; 
    auto setMSAA(bool enabled) -> void;
    [[nodiscard]] auto isMsaaEnabled() const -> bool;
    auto loadSkybox(const std::vector<std::shared_ptr<Texture>>& faceFiles, std::shared_ptr<Shader> shader) -> bool;
    auto unloadSkybox() -> void;

    auto setPreRenderCallback(std::function<void()> callback) -> void {
        m_preRenderCallback = std::move(callback);
    }
    auto setPostRenderCallback(std::function<void()> callback) -> void {
        m_postRenderCallback = std::move(callback);
    }

    [[nodiscard]] auto getDrawCallCount() const -> size_t; 
    [[nodiscard]] auto getVertexCount() const -> size_t;
    [[nodiscard]] auto getTriangleCount() const -> size_t;


   private:
    std::shared_ptr<Window> m_window;
    bool m_skyboxEnabled = false;
    bool m_vsyncEnabled = false;
    bool m_msaaEnabled = false;

    std::function<void()> m_preRenderCallback;
    std::function<void()> m_postRenderCallback;

    // statistics
    size_t m_drawCallCount = 0;
    size_t m_vertexCount = 0;
    size_t m_triangleCount = 0;

    // shadow test
    GLuint m_shadowMap;
    GLuint m_shadowFBO;

    std::shared_ptr<Shader> m_shadowShader;
};

}  // namespace Vengine