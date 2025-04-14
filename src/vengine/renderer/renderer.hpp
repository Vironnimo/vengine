#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/camera.hpp"
#include "vengine/renderer/materials.hpp"
#include "vengine/renderer/mesh.hpp"
#include "vengine/renderer/window.hpp"
#include "vengine/renderer/shaders.hpp"
#include "vengine/renderer/material.hpp"
#include "vengine/renderer/fonts.hpp"

namespace Vengine {

struct RenderObject {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
};

struct TextObject {
    std::string text;
    std::shared_ptr<Font> font;
    float x, y, scale;
    glm::vec4 color;
};

class Renderer {
    // TODO we need a scene, probably we'll just give the scene over to the renderer?
   public:
    std::unique_ptr<Materials> materials;
    std::unique_ptr<Shaders> shaders;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Fonts> fonts;

    Renderer();
    ~Renderer();
    [[nodiscard]] auto init(std::shared_ptr<Window> window) -> tl::expected<void, Error>;

    auto render(float deltaTime) -> void;
    auto setVSync(bool enabled) -> void;

    auto addRenderObject(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) -> void;
    auto addTextObject(std::shared_ptr<TextObject> textObject) -> void;

   private:
    std::shared_ptr<Window> m_window;
    std::vector<RenderObject> m_renderObjects;
    std::vector<std::shared_ptr<TextObject>> m_textObjects;
};

}  // namespace Vengine