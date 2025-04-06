#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/mesh.hpp"
#include "vengine/renderer/window.hpp"
#include "vengine/renderer/shaders.hpp"
#include "vengine/core/resource_manager.hpp"
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
    std::unique_ptr<Shaders> m_shaders;

    Renderer();
    ~Renderer();
    [[nodiscard]] auto init(std::shared_ptr<Window> window) -> tl::expected<void, Error>;

    auto render() -> void;
    auto setVSync(bool enabled) -> void;

    auto addRenderObject(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) -> void; 

   private:
    std::shared_ptr<Window> m_window;
    std::shared_ptr<Mesh> m_triangle;
    std::shared_ptr<Mesh> m_rectangle;

    // testing
    std::vector<RenderObject> m_renderObjects;
    std::shared_ptr<ResourceManager> m_resourceManager;
};

}  // namespace Vengine