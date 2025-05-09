#include "test_module.hpp"

#include <memory>
#include <string>

#include "vengine/core/uuid.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"

TestModule::TestModule() {
    spdlog::debug("Constructor TestModule");
}

TestModule::~TestModule() {
    spdlog::debug("Destructor TestModule");
}

void TestModule::onAttach(Vengine::Vengine& vengine) {
    spdlog::debug("Constructor TestModule");
    // little debug overlay
    auto textEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(textEntity, "TextEntity");
    vengine.ecs->addComponent<Vengine::PersistentComponent>(textEntity);
    vengine.ecs->addComponent<Vengine::TextComponent>(textEntity,
                                                      "Test Text",
                                                      "default",
                                                      5.0f,
                                                      625.0f,
                                                      1.0f,
                                                      glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    // subscribe to mouse moved event
    // vengine.events->subscribe<Vengine::MouseMovedEvent>([](const Vengine::MouseMovedEvent& event) {
    //     spdlog::info("Mouse: x " + std::to_string(event.x) + ", y " + std::to_string(event.y) + "\n" +
    //                  "Last Mouse: " + std::to_string(event.lastX) + ", " + std::to_string(event.lastY));
    // });
    // subscribe to key pressed and released events
    // vengine.events->subscribe<Vengine::KeyPressedEvent>([](const Vengine::KeyPressedEvent& event) {
    //     spdlog::info("Key pressed: " + std::to_string(event.key) + ", repeat: " + std::to_string(event.repeat));
    // });
    // vengine.events->subscribe<Vengine::KeyReleasedEvent>(
    //     [](const Vengine::KeyReleasedEvent& event) { spdlog::info("Key released: " + std::to_string(event.key)); });
}

void TestModule::onUpdate(Vengine::Vengine& vengine, float deltaTime) {
    // calculate fps
    if (m_testFpsTimer >= 1.0f) {
        m_testFpsTimer = 0.0f;
        m_testFps = m_testFpsCount;
        m_testFpsCount = 0;
    } else {
        m_testFpsCount++;
    }
    m_testFpsTimer += deltaTime;

    // little debug overlay
    m_fpsUpdateTimer += deltaTime;
    if (m_fpsUpdateTimer >= 0.1f) {
        m_fpsUpdateTimer = 0.0f;
        int fps = static_cast<int>(1.0f / deltaTime);
        auto text = "Scene: " + vengine.getCurrentSceneName() + "\nDeltaTime: " + std::to_string(deltaTime) +
                    "\nDeltaTime FPS: " + std::to_string(fps) + "\n" + "Counter FPS: " + std::to_string(m_testFps) + "\n" +
                    "Entity count: " + std::to_string(vengine.ecs->getEntityCount()) + "\n" + Vengine::UUID::toString();

        auto textEntity = vengine.ecs->getEntityByTag("TextEntity");
        auto textComp = vengine.ecs->getEntityComponent<Vengine::TextComponent>(textEntity.getId());
        if (textComp) {
            textComp->text = text;
        }
    }

    // play sound on hitting the ground. this is only working once, as long as we do it inside a module
    auto rigidBody = vengine.ecs->getComponentByEntityTag<Vengine::RigidbodyComponent>("chair");
    if (rigidBody && rigidBody->isGrounded) {
        if (m_soundFirstClick) {
            // TODO doesn't work when we reload the scene. entities differ. we need a string id on the entitites to find them
            // by name
            vengine.resourceManager->get<Vengine::Sound>("click")->play();
            m_soundFirstClick = false;
        }
    }
}

void TestModule::onDetach(Vengine::Vengine& vengine) {
    (void)vengine;  // haha clang tidy
}