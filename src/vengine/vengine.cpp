#include "vengine.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <tl/expected.hpp>

#include "vengine/core/event_manager.hpp"
#include "vengine/core/thread_manager.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/renderer/renderer.hpp"
#include "vengine/core/resource_manager.hpp"
#include "vengine/ecs/systems.hpp"
#include "vengine/core/input_manager.hpp"
#include "vengine/core/cameras.hpp"

namespace Vengine {

Vengine::Vengine() {
    auto result = init();
    if (!result) {
        spdlog::error("{}", result.error().toString());
    }
}

Vengine::~Vengine() {
    spdlog::debug("Destructor Vengine");

    // so what would be the best order to destroy things?
    g_eventManager.clear();
    threadManager.reset();
    resourceManager.reset();
    scenes.reset();
    m_modules.clear();
    actions.reset();
    renderer.reset();
    signals.reset();
    timers.reset();
    inputSystem.reset();
    window.reset();
    ecs.reset();
}

[[nodiscard]] auto Vengine::init() -> tl::expected<void, Error> {
    timers = std::make_unique<Timers>();
    timers->start("vengine.start");

    // spdlog stuff
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
    spdlog::info("Vengine: starting...");

    signals = std::make_unique<Signals>();
    events = &g_eventManager;

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

    // resources
    resourceManager = std::make_unique<ResourceManager>();
    if (auto result = resourceManager->init(threadManager); !result) {
        return tl::unexpected(result.error());
    }

    inputSystem = std::make_unique<InputManager>();
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
    spdlog::info("Vengine: renderer initialized");
    addDefaults();
    renderer->initFonts(resourceManager->get<Shader>("default.text"));
    // if (auto result = renderer->initFonts(resourceManager->get<Shader>("default.text")); !result) {
        // spdlog::warn(result.error().message);
    // }
    renderer->setShadowShader(resourceManager->get<Shader>("shadow_depth"));

    // register built-in components
    ecs->registerComponent<TagComponent>("Tag");
    ecs->registerComponent<TextComponent>("Text");
    ecs->registerComponent<PersistentComponent>("Persistent");
    ecs->registerComponent<TransformComponent>("Transform");
    ecs->registerComponent<VelocityComponent>("Velocity");
    ecs->registerComponent<MeshComponent>("Mesh");
    ecs->registerComponent<ModelComponent>("Model");
    ecs->registerComponent<MaterialComponent>("Material");
    ecs->registerComponent<ScriptComponent>("Script");
    ecs->registerComponent<CameraComponent>("Camera");
    ecs->registerComponent<PhysicsComponent>("Physics");
    ecs->registerComponent<LightComponent>("Light");
    // register built-in systems
    auto transformSystem = std::make_shared<TransformSystem>();
    transformSystem->setEnabled(false);  // calling manually to make sure it runs before collision and physics
    auto physicsSystem = std::make_shared<PhysicsSystem>();
    physicsSystem->setEnabled(false);  // calling manually to make sure it runs before collision and physics
    ecs->registerSystem("TransformSystem", std::make_shared<TransformSystem>());
    ecs->registerSystem("PhysicsSystem", physicsSystem);
    auto scriptSystem = std::make_shared<ScriptSystem>();
    scriptSystem->setEnabled(false);  // calling manually to make sure it runs before collision and physics
    scriptSystem->registerBindings(this);
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

        threadManager->processMainThreadTasks();
        timers->update();
        inputSystem->update();

        for (auto& module : m_modules) {
            module->onUpdate(*this, timers->deltaTime());
        }
        actions->handleInput(window->get());

        // script system before everything else?
        auto scriptSystem = ecs->getSystem<ScriptSystem>("ScriptSystem");
        scriptSystem->update(ecs->getActiveEntities(), timers->deltaTime());

        auto physicsSystem = ecs->getSystem<PhysicsSystem>("PhysicsSystem");
        physicsSystem->update(ecs->getActiveEntities(), timers->deltaTime());

        auto transformSystem = ecs->getSystem<TransformSystem>("TransformSystem");
        transformSystem->update(ecs->getActiveEntities(), timers->deltaTime());

        // auto physicsAndCollisionTaskFuture = threadManager->enqueueTask(
        //     [this]() {
        //         auto joltPhysicsSystem = ecs->getSystem<JoltPhysicsSystem>("JoltPhysicsSystem");
        //         joltPhysicsSystem->update(ecs->getActiveEntities(), timers->deltaTime());
        //     },
        //     "Physics and Collision System");
        // physicsAndCollisionTaskFuture.wait();

        ecs->runSystems(timers->deltaTime());
        if (scenes->getCurrentScene() == nullptr) {
            // spdlog::warn("Vengine: No current scene set, skipping rendering.");
            continue;
        }
        renderer->render(scenes->getCurrentScene());
    }
}

void Vengine::addDefaults() const {
    // default resources
    resourceManager->load<Shader>("default", "resources/shaders/default_new.vert", "resources/shaders/default_new.frag");
    resourceManager->load<Shader>("default.text", "resources/shaders/text.vert", "resources/shaders/text.frag");
    resourceManager->load<Shader>("shadow_depth",
                                  "resources/shaders/shadow_depth.vert",
                                  "resources/shaders/shadow_depth.frag");
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
        g_eventManager.publish(WindowResizeEvent{width, height});
        glViewport(0, 0, width, height);
        auto* vengine = static_cast<Vengine*>(glfwGetWindowUserPointer(wnd));
        if (vengine && vengine->ecs) {
            auto camComp = vengine->ecs->getEntityComponent<CameraComponent>(
                vengine->scenes->getCurrentScene()->getCameras()->getActive());
            camComp->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        }
    });

    glfwSetKeyCallback(window->get(), [](GLFWwindow*, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            g_eventManager.publish(KeyPressedEvent{key, false});
        } else if (action == GLFW_REPEAT) {
            g_eventManager.publish(KeyPressedEvent{key, true});
        } else if (action == GLFW_RELEASE) {
            g_eventManager.publish(KeyReleasedEvent{key});
        }
    });

    // subscribe to camera changed event
    // TODO gotta rethink this, and obviously change some stuff inside
    // do i even need the camera changed event here? or can i now just set the scroll callback
    events->subscribe<CameraChangedEvent>([this](const CameraChangedEvent& event) {
        spdlog::debug("Vengine: Active Camera changed to CameraID: {}", event.newCamera);

        glfwSetScrollCallback(window->get(), [](GLFWwindow*, double xoffset, double yoffset) {
            g_eventManager.publish(MouseScrollEvent{static_cast<int>(xoffset), static_cast<int>(yoffset)});
        });
    });

    // TODO still need to handle the cursor pos callback, need access to last mouse pos,
    // is the userpointer in renderer not good?
    // glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
    //     g_eventSystem.publish(MouseMovedEvent{static_cast<int>(xpos), static_cast<int>(ypos), 0, 0});
    // });
}

}  // namespace Vengine