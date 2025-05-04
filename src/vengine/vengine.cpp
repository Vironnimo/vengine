#include "vengine.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/input_system.hpp"
#include "vengine/core/thread_manager.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"
#include "vengine/ecs/systems.hpp"

namespace Vengine {

Vengine::Vengine() {
    auto result = init();
    if (!result) {
        spdlog::error("{}", result.error().message);
    }
}

Vengine::~Vengine() {
    spdlog::debug("Destructor Vengine");
    // making sure resource manager is destroyed last
    m_scenes.reset();
    meshLoader.reset();
    actions.reset();
    renderer.reset();
    events.reset();
    timers.reset();
    ecs.reset();
    m_modules.clear();
    window.reset();
    resourceManager.reset();
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
    auto windowCreationTime = timers->getElapsed("vengine.window_creation");

    threadManager = std::make_shared<ThreadManager>();

    resourceManager = std::make_unique<ResourceManager>(threadManager);
    if (auto result = resourceManager->init(); !result) {
        return tl::unexpected(result.error());
    }

    inputSystem = std::make_unique<InputSystem>();
    inputSystem->setWindow(window->get());
    actions = std::make_unique<Actions>();
    meshLoader = std::make_unique<MeshLoader>();

    m_scenes = std::make_unique<Scenes>();

    // ecs
    ecs = std::make_shared<ECS>();
    // renderer
    renderer = std::make_unique<Renderer>();
    if (auto result = renderer->init(window, ecs); !result) {
        return tl::unexpected(result.error());
    }

    // register built-in components
    ecs->registerComponent<TagComponent>("Tag");
    ecs->registerComponent<PersistentComponent>("Persistent");
    ecs->registerComponent<TransformComponent>("Transform");
    ecs->registerComponent<MeshComponent>("Mesh");
    ecs->registerComponent<MaterialComponent>("Material");
    ecs->registerComponent<RigidbodyComponent>("Rigidbody");
    ecs->registerComponent<ColliderComponent>("Collider");
    ecs->registerComponent<PositionComponent>("Position");
    ecs->registerComponent<VelocityComponent>("Velocity");
    ecs->registerComponent<ScriptComponent>("Script");
    ecs->registerComponent<CameraComponent>("Camera");
    // register built-in systems
    auto renderSystem = std::make_shared<RenderSystem>();
    renderSystem->setEnabled(false);  // because it's not called automatically, it's called manually by the renderer
    auto collisionSystem = std::make_shared<CollisionSystem>();
    auto physicsSystem = std::make_shared<PhysicsSystem>();
    auto transformSystem = std::make_shared<TransformSystem>();
    transformSystem->setEnabled(false);  // calling manually to make sure it runs before collision and physics
    collisionSystem->setEnabled(false);  // these two run on an extra thread
    physicsSystem->setEnabled(false);
    ecs->registerSystem("MovementSystem", std::make_shared<MovementSystem>());
    ecs->registerSystem("TransformSystem", std::make_shared<TransformSystem>());
    ecs->registerSystem("CollisionSystem", collisionSystem);
    ecs->registerSystem("PhysicsSystem", physicsSystem);
    ecs->registerSystem("RenderSystem", renderSystem);
    auto scriptSystem = std::make_shared<ScriptSystem>();
    scriptSystem->registerBindings(ecs);
    ecs->registerSystem("ScriptSystem", scriptSystem);

    // default cam
    EntityId defaultCameraEntity = ecs->createEntity();
    ecs->addComponent<TagComponent>(defaultCameraEntity, "DefaultCamera");  
    ecs->addComponent<PersistentComponent>(defaultCameraEntity); 
    ecs->addComponent<TransformComponent>(defaultCameraEntity);
    ecs->addComponent<CameraComponent>(defaultCameraEntity); 
    ecs->addComponent<ScriptComponent>(defaultCameraEntity, "resources/scripts/camera.lua");

    auto camComp = ecs->getEntityComponent<CameraComponent>(defaultCameraEntity);
    camComp->aspectRatio = static_cast<float>(params.width) / static_cast<float>(params.height);

    auto camTransform = ecs->getEntityComponent<TransformComponent>(defaultCameraEntity); 
    camTransform->position = glm::vec3(0.0f, 50.0f, 185.0f);  

    renderer->setActiveCamera(defaultCameraEntity);

    // time logging
    auto vengineStartTime = timers->getElapsed("vengine.start");
    auto vengineWithoutWindow = vengineStartTime - windowCreationTime;
    spdlog::info("Vengine: started in {} ms.", vengineStartTime);
    spdlog::info("Vengine: -- {} ms of that are vengine", vengineWithoutWindow);
    spdlog::info("Vengine: -- {} ms of that are window creation", windowCreationTime);
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
        threadManager->processMainThreadTasks();
        inputSystem->update();

        for (auto& module : m_modules) {
            module->onUpdate(*this, timers->deltaTime());
        }
        actions->handleInput(window->get());

        auto transformSystem = ecs->getSystem<TransformSystem>("TransformSystem");
        transformSystem->update(ecs->getActiveEntities(), timers->deltaTime());

        auto physicsTaskFuture = threadManager->enqueueTask(
            [this]() {
                auto collisionSystem = ecs->getSystem<CollisionSystem>("CollisionSystem");
                auto physicsSystem = ecs->getSystem<PhysicsSystem>("PhysicsSystem");
                if (collisionSystem) {
                    collisionSystem->update(ecs->getActiveEntities(), timers->deltaTime());
                }
                if (physicsSystem) {
                    physicsSystem->update(ecs->getActiveEntities(), timers->deltaTime());
                }
            },
            "Physics and Collision System");
        physicsTaskFuture.wait();

        ecs->runSystems(timers->deltaTime());
        renderer->render(ecs, timers->deltaTime());
    }
}

void Vengine::addModule(std::shared_ptr<Module> module) {
    module->onAttach(*this);
    m_modules.push_back(std::move(module));
}

void Vengine::removeModule(const std::shared_ptr<Module>& module) {
    auto it = std::find(m_modules.begin(), m_modules.end(), module);
    if (it != m_modules.end()) {
        (*it)->onDetach(*this);
        m_modules.erase(it);
    }
}

void Vengine::addScene(const std::string& name, std::shared_ptr<Scene> scene) {
    m_scenes->add(name, std::move(scene));
}

template <typename T>
void Vengine::addScene(const std::string& name) {
    auto scene = std::make_shared<T>(name);
}

void Vengine::switchToScene(const std::string& name) {
    m_scenes->switchTo(name, *this);
}

void Vengine::removeScene(const std::string& name) {
    m_scenes->remove(name);
}

}  // namespace Vengine