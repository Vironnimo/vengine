#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/renderer/mesh_loader.hpp"
#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"
#include "vengine/core/actions.hpp"
#include "vengine/core/event_system.hpp"
#include "vengine/core/timers.hpp"
#include "vengine/core/layer.hpp"
#include "vengine/ecs/ecs.hpp"

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
    std::unique_ptr<Timers> timers;
    std::shared_ptr<ECS> ecs;

    Vengine();
    ~Vengine();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;

    void addLayer(std::shared_ptr<Layer> layer);
    void removeLayer(std::shared_ptr<Layer> layer);

    auto run() -> void;

   private:
    std::vector<std::shared_ptr<Layer>> m_layers;
};

}  // namespace Vengine