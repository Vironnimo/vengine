#include "test_module.hpp"

#include <memory>
#include <string>

#include "vengine/core/uuid.hpp"
#include "vengine/vengine.hpp"

void TestModule::onAttach(Vengine::Vengine& vengine) {
    spdlog::debug("Constructor TestModule");
    // little debug overlay
    m_textObject = std::make_shared<Vengine::TextObject>();
    m_textObject->text = "FPS: 0";
    m_textObject->font = vengine.renderer->fonts->get("default").value();
    m_textObject->x = 25.0f;
    m_textObject->y = 425.0f;
    m_textObject->scale = 1.0f;
    m_textObject->color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    vengine.renderer->addTextObject(m_textObject);
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
        m_textObject->text = "Scene: " + vengine.getCurrentSceneName() + "\nDeltaTime: " + std::to_string(deltaTime) +
                             "\nDeltaTime FPS: " + std::to_string(fps) + "\n" + "Counter FPS: " + std::to_string(m_testFps) + "\n" +
                             "Entity count: " + std::to_string(vengine.ecs->getEntityCount()) + "\n" + Vengine::UUID::toString();
    }

    static bool firstClick = true;
    auto rigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(2);
    if (rigidBody && rigidBody->isGrounded) {
        if (firstClick) {
            // doesn't work when we reload the scene. entities differ. we need a string id on the entitites to find them by name
            vengine.resourceManager->get<Vengine::Sound>("click")->play();
            firstClick = false;
        }
    } else {
        firstClick = true;
    }
}

void TestModule::onDetach(Vengine::Vengine& vengine) {
}