#include "vengine.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <tl/expected.hpp>

#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"

namespace Vengine {

Vengine::Vengine() {
    auto result = init();
    if (!result) {
        spdlog::error("{}", result.error().message);
    }
}

Vengine::~Vengine() {
    spdlog::debug("Destructor Vengine");
}

[[nodiscard]] auto Vengine::init() -> tl::expected<void, Error> {
    timers = std::make_unique<Timers>();
    timers->start("vengine.start");

    // spdlog stuff
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
    spdlog::info("Vengine: starting...");

    events = std::make_unique<EventSystem>();

    timers->start("vengine.window_creation");
    window = std::make_shared<Window>();
    if (auto result = window->init(); !result) {
        return tl::unexpected(result.error());
    }
    WindowParams params;
    if (auto result = window->create(params); !result) {
        return tl::unexpected(result.error());
    }
    spdlog::info("Vengine: window creation time: {} ms", timers->stop("vengine.window_creation"));

    renderer = std::make_unique<Renderer>();
    if (auto result = renderer->init(window); !result) {
        return tl::unexpected(result.error());
    }

    resourceManager = std::make_unique<ResourceManager>();
    if (auto result = resourceManager->init(); !result) {
        return tl::unexpected(result.error());
    }

    actions = std::make_unique<Actions>();
    meshLoader = std::make_unique<MeshLoader>();

    m_scenes = std::make_unique<Scenes>();

    // ecs
    ecs = std::make_shared<ECS>();
    // NOTE: add default systems
    auto renderSystem = std::make_shared<RenderSystem>(renderer->camera);
    renderSystem->setEnabled(false);  // because it's not called automatically, it's called manually by the renderer
    ecs->registerSystem("MovementSystem", std::make_shared<MovementSystem>());
    ecs->registerSystem("TransformSystem", std::make_shared<TransformSystem>());
    ecs->registerSystem("CollisionSystem", std::make_shared<CollisionSystem>());
    ecs->registerSystem("PhysicsSystem", std::make_shared<PhysicsSystem>());
    ecs->registerSystem("RenderSystem", renderSystem);

    // this is weird here, needs to move
    glfwSetWindowUserPointer(window->get(), this);

    // vengine startup time
    auto elapsedTime = timers->getElapsed("vengine.start");
    spdlog::info("Vengine: initialization took {} ms", elapsedTime);

    spdlog::info("Vengine: successfully started.");
    return {};
}

auto Vengine::run() -> void {
    spdlog::info("Vengine: run");
    isRunning = true;

    while (isRunning) {
        if (glfwWindowShouldClose(window->get()) == 1) {
            isRunning = false;
            break;
        }

        timers->update();
        for (auto& module : m_modules) {
            module->onUpdate(timers->deltaTime());
        }
        actions->handleInput(window->get());
        ecs->runSystems(timers->deltaTime());
        renderer->render(ecs, timers->deltaTime());
    }
}

void Vengine::addModule(std::shared_ptr<Module> module) {
    module->onAttach();
    m_modules.push_back(std::move(module));
}

void Vengine::removeModule(std::shared_ptr<Module> module) {
    auto it = std::find(m_modules.begin(), m_modules.end(), module);
    if (it != m_modules.end()) {
        (*it)->onDetach();
        m_modules.erase(it);
    }
}

void Vengine::addScene(const std::string& name, std::shared_ptr<Scene> scene) {
    m_scenes->add(name, std::move(scene), ecs->createEntitySet(name));
}

template <typename T>
void Vengine::addScene(const std::string& name) {
    auto scene = std::make_shared<T>(name);
    m_scenes->add(name, std::move(scene), ecs->createEntitySet(name));
}

void Vengine::switchToScene(const std::string& name) {
    m_scenes->switchTo(name);
}

void Vengine::removeScene(const std::string& name) {
    m_scenes->remove(name);
}

}  // namespace Vengine