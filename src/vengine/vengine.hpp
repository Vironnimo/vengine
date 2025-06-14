#pragma once

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/error.hpp"
#include "vengine/core/thread_manager.hpp"
#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"
#include "vengine/core/actions.hpp"
#include "vengine/core/signals.hpp"
#include "vengine/core/event_manager.hpp"
#include "vengine/core/timers.hpp"
#include "vengine/core/module.hpp"
#include "vengine/ecs/ecs.hpp"
#include "vengine/core/scenes.hpp"
#include "vengine/core/input_manager.hpp"

namespace Vengine {

class InputManager;

class Vengine {
   public:
    bool isRunning = false;
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<Window> window;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<InputManager> inputSystem;
    std::unique_ptr<Actions> actions;
    std::unique_ptr<Signals> signals;
    std::unique_ptr<Timers> timers;
    std::shared_ptr<ECS> ecs;
    std::shared_ptr<ThreadManager> threadManager;
    EventManager* events = nullptr;
    std::unique_ptr<Scenes> scenes;

    Vengine();
    ~Vengine();
    [[nodiscard]] auto init() -> tl::expected<void, Error>;

    void addModule(std::shared_ptr<Module> module);
    void removeModule(const std::shared_ptr<Module>& module);

    void addScene(const std::string& name, std::shared_ptr<Scene> scene) const;
    template <typename T>
    void addScene(const std::string& name) {
        auto scene = std::make_shared<T>(name);
        scenes->add(name, std::move(scene));
    }

    void loadScene(const std::string& name);
    void removeScene(const std::string& name) const;
    [[nodiscard]] auto getCurrentSceneName() const -> std::string {
        return scenes->getCurrentSceneName();
    }

    auto run() -> void;

   private:
    std::vector<std::shared_ptr<Module>> m_modules;

    void addDefaults() const;
    void registerGlfwCallbacks();
};

}  // namespace Vengine