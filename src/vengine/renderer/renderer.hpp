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

namespace Vengine {

struct RenderObject {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
};

class Renderer {
    // TODO we need viewport or what its called for opengl, our base camera right?
    // TODO we need a scene, probably we'll just give the scene over to the renderer?
   public:
    std::unique_ptr<Materials> materials;
    std::unique_ptr<Shaders> shaders;

    Renderer();
    ~Renderer();
    [[nodiscard]] auto init(std::shared_ptr<Window> window) -> tl::expected<void, Error>;

    auto render() -> void;
    auto setVSync(bool enabled) -> void;

    auto addRenderObject(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) -> void; 

   private:
    std::shared_ptr<Window> m_window;
    std::unique_ptr<Camera> m_camera;

    std::vector<RenderObject> m_renderObjects;
};

}  // namespace Vengine