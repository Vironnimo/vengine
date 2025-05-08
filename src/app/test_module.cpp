#include "test_module.hpp"

#include <memory>
#include <string>

#include "vengine/core/uuid.hpp"
#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"

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
                             "\nDeltaTime FPS: " + std::to_string(fps) + "\n" + "Counter FPS: " + std::to_string(m_testFps) +
                             "\n" + "Entity count: " + std::to_string(vengine.ecs->getEntityCount()) + "\n" +
                             Vengine::UUID::toString();

        auto textEntity = vengine.ecs->getEntityByTag("TextEntity");
        auto textComp = vengine.ecs->getEntityComponent<Vengine::TextComponent>(textEntity.getId());
        if (textComp) {
            textComp->text = text;
        }
    }

    static bool firstClick = true;
    auto rigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(2);
    if (rigidBody && rigidBody->isGrounded) {
        if (firstClick) {
            // TODO doesn't work when we reload the scene. entities differ. we need a string id on the entitites to find them
            // by name
            vengine.resourceManager->get<Vengine::Sound>("click")->play();
            firstClick = false;
        }
    } else {
        firstClick = true;
    }

    // camera movement, here for testing
    auto cam = vengine.ecs->getEntityByTag("MainCamera");
    auto camTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(cam.getId());
    const float cameraSpeed = 100.0f;
    const float mouseSensitivity = 0.01f;

    if (camTransform) {
        float yaw = camTransform->getRotationY();
        // float pitch = glm::radians(camTransform->rotation.x);

        // yap, something's being done here.
        glm::vec3 forward = glm::normalize(glm::vec3(std::sin(yaw), 0.0f, -std::cos(yaw)));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (vengine.inputSystem->isKeyDown(GLFW_KEY_E)) {
            camTransform->setPosition(camTransform->getPosition() + forward * cameraSpeed * deltaTime);
        }
        if (vengine.inputSystem->isKeyDown(GLFW_KEY_D)) {
            camTransform->setPosition(camTransform->getPosition() - forward * cameraSpeed * deltaTime);
        }
        if (vengine.inputSystem->isKeyDown(GLFW_KEY_S)) {
            camTransform->setPosition(camTransform->getPosition() - right * cameraSpeed * deltaTime);
        }
        if (vengine.inputSystem->isKeyDown(GLFW_KEY_F)) {
            camTransform->setPosition(camTransform->getPosition() + right * cameraSpeed * deltaTime);
        }
        if (vengine.inputSystem->isKeyDown(GLFW_KEY_BACKSPACE)) {
            camTransform->setPosition(camTransform->getPosition() + glm::vec3(0.0f, cameraSpeed * deltaTime, 0.0f));
        }
        if (vengine.inputSystem->isKeyDown(GLFW_KEY_DELETE)) {
            camTransform->setPosition(camTransform->getPosition() - glm::vec3(0.0f, cameraSpeed * deltaTime, 0.0f));
        }

        if (vengine.inputSystem->isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT)) {
            auto deltaX = static_cast<float>(vengine.inputSystem->getMouseDeltaX());
            auto deltaY = static_cast<float>(vengine.inputSystem->getMouseDeltaY());

            glm::vec3 rotation = camTransform->getRotation();
            rotation.x -= -deltaY * mouseSensitivity;
            rotation.y -= -deltaX * mouseSensitivity;
            // avoid flipping over the top and bottom
            rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);
            camTransform->setRotation(rotation);
        }
    }
}

void TestModule::onDetach(Vengine::Vengine& vengine) {
    (void)vengine;  // haha clang tidy
}