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
#include "vengine/core/module.hpp"
#include "vengine/ecs/ecs.hpp"
#include "vengine/core/scenes.hpp"

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

    void addModule(std::shared_ptr<Module> module);
    void removeModule(std::shared_ptr<Module> module);

    // TODO: move this somewhere else i guess
    void addScene(const std::string& name, std::shared_ptr<Scene> scene);

    template <typename T>
    void addScene(const std::string& name);

    void switchToScene(const std::string& name);
    void removeScene(const std::string& name);

    auto run() -> void;

   private:
    std::unique_ptr<Scenes> m_scenes;
    std::vector<std::shared_ptr<Module>> m_modules;
};

}  // namespace Vengine