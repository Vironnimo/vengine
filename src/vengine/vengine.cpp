#include "vengine.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/thread_manager.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"
#include "vengine/ecs/systems.hpp"
#include "vengine/core/input_system.hpp"
#include "vengine/core/cameras.hpp"

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
    threadManager->waitForCompletion();

    threadManager.reset();
    scenes.reset();
    m_modules.clear();
    actions.reset();
    renderer.reset();
    signals.reset();
    timers.reset();
    spdlog::debug("ECS use count before reset: {}", ecs.use_count());
    ecs.reset();
    spdlog::debug("ECS use count after reset: {}", ecs.use_count());
    inputSystem.reset();
    window.reset();
    resourceManager.reset();
    // spdlog::debug("ECS use count before reset: {}", ecs.use_count());
}

[[nodiscard]] auto Vengine::init() -> tl::expected<void, Error> {
    timers = std::make_unique<Timers>();
    timers->start("vengine.start");

    // spdlog stuff
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
    spdlog::info("Vengine: starting...");

    signals = std::make_unique<SignalSystem>();
    events = &g_eventSystem;

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

    // glfw callbacks
    registerGlfwCallbacks();

    threadManager = std::make_shared<ThreadManager>();

    resourceManager = std::make_unique<ResourceManager>(threadManager);
    if (auto result = resourceManager->init(); !result) {
        return tl::unexpected(result.error());
    }

    inputSystem = std::make_unique<InputSystem>();
    inputSystem->setWindow(window->get());
    actions = std::make_unique<Actions>();

    scenes = std::make_unique<Scenes>();

    // ecs
    ecs = std::make_shared<ECS>();
    // renderer
    renderer = std::make_unique<Renderer>();
    if (auto result = renderer->init(window); !result) {
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
    ecs->registerComponent<ScriptComponent>("Script");
    ecs->registerComponent<CameraComponent>("Camera");
    // register built-in systems
    auto collisionSystem = std::make_shared<CollisionSystem>();
    auto physicsSystem = std::make_shared<PhysicsSystem>();
    auto transformSystem = std::make_shared<TransformSystem>();
    transformSystem->setEnabled(false);  // calling manually to make sure it runs before collision and physics
    collisionSystem->setEnabled(false);  // these two run on an extra thread
    physicsSystem->setEnabled(false);
    ecs->registerSystem("TransformSystem", std::make_shared<TransformSystem>());
    ecs->registerSystem("CollisionSystem", collisionSystem);
    ecs->registerSystem("PhysicsSystem", physicsSystem);
    auto scriptSystem = std::make_shared<ScriptSystem>();
    scriptSystem->registerBindings(ecs);
    ecs->registerSystem("ScriptSystem", scriptSystem);

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
        inputSystem->update();

        for (auto& module : m_modules) {
            module->onUpdate(*this, timers->deltaTime());
        }
        actions->handleInput(window->get());
        threadManager->processMainThreadTasks();

        auto transformSystem = ecs->getSystem<TransformSystem>("TransformSystem");
        transformSystem->update(ecs->getActiveEntities(), timers->deltaTime());

        auto physicsAndCollisionTaskFuture = threadManager->enqueueTask(
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
        physicsAndCollisionTaskFuture.wait();

        ecs->runSystems(timers->deltaTime());
        renderer->render(scenes->getCurrentScene());
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

void Vengine::addScene(const std::string& name, std::shared_ptr<Scene> scene) const {
    scenes->add(name, std::move(scene));
}

template <typename T>
void Vengine::addScene(const std::string& name) {
    auto scene = std::make_shared<T>(name);
}

void Vengine::loadScene(const std::string& name) {
    scenes->load(name, *this);
}

void Vengine::removeScene(const std::string& name) const {
    scenes->remove(name);
}

void Vengine::registerGlfwCallbacks() {
    // TODO rethink all of this
    // user pointer
    glfwSetWindowUserPointer(window->get(), this);

    // should just trigger a window resize event, right?
    glfwSetFramebufferSizeCallback(window->get(), [](GLFWwindow* wnd, int width, int height) {
        // TODO so where do we handle the resize event?
        g_eventSystem.publish(WindowResizeEvent{width, height});
        glViewport(0, 0, width, height);
        auto* vengine = static_cast<Vengine*>(glfwGetWindowUserPointer(wnd));
        if (vengine && vengine->ecs) {
            auto camComp = vengine->ecs->getEntityComponent<CameraComponent>(vengine->scenes->getCurrentScene()->getCameras()->getActive());
            camComp->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        }
    });

    glfwSetKeyCallback(window->get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            g_eventSystem.publish(KeyPressedEvent{key, false});
        } else if (action == GLFW_REPEAT) {
            g_eventSystem.publish(KeyPressedEvent{key, true});
        } else if (action == GLFW_RELEASE) {
            g_eventSystem.publish(KeyReleasedEvent{key});
        }
    });

    // subscribe to camera changed event
    // TODO gotta rethink this, and obviously change some stuff inside
    events->subscribe<CameraChangedEvent>([this](const CameraChangedEvent& event) {
        spdlog::debug("Vengine: Active Camera changed to CameraID: {}", event.newCamera);

        glfwSetScrollCallback(window->get(), [](GLFWwindow* wnd, double, double yoffset) {
            yoffset *= 2.0;  // sens
            auto* vengine = static_cast<Vengine*>(glfwGetWindowUserPointer(wnd));
            if (!vengine || vengine->scenes->getCurrentScene()->getCameras()->getActive() == 0) {
                return;
            }
            auto cameraTransform = vengine->ecs->getEntityComponent<TransformComponent>(vengine->scenes->getCurrentScene()->getCameras()->getActive());
            auto camComp = vengine->ecs->getEntityComponent<CameraComponent>(vengine->scenes->getCurrentScene()->getCameras()->getActive());
            if (camComp && cameraTransform) {
                camComp->fov -= static_cast<float>(yoffset);
                // TODO: max fov somewhere else? and not hardcoded...
                if (camComp->fov < 1.0f) {
                    camComp->fov = 1.0f;
                }
                if (camComp->fov > 90.0f) {
                    camComp->fov = 90.0f;
                }
            }
        });
    });

    // TODO still need to handle the cursor pos callback, need access to last mouse pos,
    // is the userpointer in renderer not good?
    // glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
    //     g_eventSystem.publish(MouseMovedEvent{static_cast<int>(xpos), static_cast<int>(ypos), 0, 0});
    // });
}

}  // namespace Vengine