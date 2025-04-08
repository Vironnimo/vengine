#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/mesh_loader.hpp"
#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"

namespace Vengine {

class Vengine {
   public:
    bool isRunning = false;
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<Window> window;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<MeshLoader> meshLoader;

    Vengine();
    ~Vengine();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;

    auto run() -> void;

   private:
};

}  // namespace Vengine