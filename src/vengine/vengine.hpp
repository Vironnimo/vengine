#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/mesh_loader.hpp"
#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"
#include "vengine/core/actions.hpp"
#include "vengine/core/event_system.hpp"
#include "vengine/core/timer.hpp"

namespace Vengine {

class Vengine {
   public:
    bool isRunning = false;
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<Window> window;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<MeshLoader> meshLoader;
    std::unique_ptr<Actions> actions;
    std::unique_ptr<EventSystem> events;
    std::unique_ptr<Timer> timer;

    Vengine();
    ~Vengine();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;

    auto run() -> void;

   private:
};

}  // namespace Vengine