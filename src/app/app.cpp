#include "app.hpp"
#include <GLFW/glfw3.h>

#include <memory>

#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"
#include "vengine/renderer/renderer.hpp"

App::App() {
    m_vengine = std::make_shared<Vengine::Vengine>();
    m_vengine->timers->start("app_constructor");

    m_vengine->renderer->camera->setPosition(glm::vec3(0.0f, 20.0f, 205.0f));
    // -------------------- ACTIONS ---------------------
    m_vengine->actions->add("quit", "Quit", [this]() { m_vengine->isRunning = false; });
    m_vengine->actions->addKeybinding("quit", {GLFW_KEY_ESCAPE, false, false, false});

    // lets add actions to move the camera
    const float cameraSpeed = 100.0f;
    m_vengine->actions->add("camera.move.right", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() + glm::vec3(cameraSpeed * m_vengine->timers->deltaTime(), 0.0f, 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.right", {GLFW_KEY_F, false, false, false});

    m_vengine->actions->add("camera.move.left", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() + glm::vec3(-cameraSpeed * m_vengine->timers->deltaTime(), 0.0f, 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.left", {GLFW_KEY_S, false, false, false});

    m_vengine->actions->add("camera.move.up", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() + glm::vec3(0.0f, cameraSpeed * m_vengine->timers->deltaTime(), 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.up", {GLFW_KEY_BACKSPACE, false, false, false});

    m_vengine->actions->add("camera.move.down", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() + glm::vec3(0.0f, -cameraSpeed * m_vengine->timers->deltaTime(), 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.move.down", {GLFW_KEY_DELETE, false, false, false});

    m_vengine->actions->add("camera.move.forward", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() + glm::vec3(0.0f, 0.0f, -cameraSpeed * m_vengine->timers->deltaTime()));
    });
    m_vengine->actions->addKeybinding("camera.move.forward", {GLFW_KEY_E, false, false, false});

    m_vengine->actions->add("camera.move.backward", "Move Camera", [this, cameraSpeed]() {
        m_vengine->renderer->camera->setPosition(m_vengine->renderer->camera->getPosition() + glm::vec3(0.0f, 0.0f, cameraSpeed * m_vengine->timers->deltaTime()));
    });
    m_vengine->actions->addKeybinding("camera.move.backward", {GLFW_KEY_D, false, false, false});

    // zoom in and out with scrollwheel
    m_vengine->actions->add("camera.zoom.in", "Zoom In",
                            [this]() { m_vengine->renderer->camera->setFov(m_vengine->renderer->camera->getFov() - 1.0f); });
    m_vengine->actions->addKeybinding("camera.zoom.in", {GLFW_KEY_UP, false, false, false});
    m_vengine->actions->add("camera.zoom.out", "Zoom Out",
                            [this]() { m_vengine->renderer->camera->setFov(m_vengine->renderer->camera->getFov() + 1.0f); });
    m_vengine->actions->addKeybinding("camera.zoom.out", {GLFW_KEY_DOWN, false, false, false});

    // this is some weird turning with the mouse..
    // we might need/want a mouse class
    m_vengine->actions->add("camera.rotate", "Turn Camera", [this]() {
        double xpos;
        double ypos;
        glfwGetCursorPos(m_vengine->window->get(), &xpos, &ypos);

        static double lastX = xpos;
        static double lastY = ypos;
        auto xoffset = static_cast<float>(xpos - lastX);
        auto yoffset = static_cast<float>(lastY - ypos);
        yoffset = -yoffset;
        lastX = xpos;
        lastY = ypos;

        m_vengine->renderer->camera->setRotation(m_vengine->renderer->camera->getRotation() +
                                                 glm::vec3(yoffset * 0.01f, xoffset * 0.01f, 0.0f));
    });
    m_vengine->actions->addKeybinding("camera.rotate", {GLFW_MOUSE_BUTTON_LEFT, false, false, false});

    // load textures
    m_vengine->resourceManager->load<Vengine::Texture>("test_texture", "test.jpg");
    auto texture = m_vengine->resourceManager->get<Vengine::Texture>("test_texture");
    m_vengine->resourceManager->load<Vengine::Texture>("test_texture2", "test2.jpg");
    auto texture2 = m_vengine->resourceManager->get<Vengine::Texture>("test_texture2");

    // create shaders
    m_vengine->renderer->shaders->add(
        std::make_shared<Vengine::Shader>("default", "resources/shaders/default.vert", "resources/shaders/default.frag"));
    // m_vengine->renderer->shaders->add(std::make_shared<Vengine::Shader>("colored", "resources/shaders/vertex.glsl",
    // "resources/shaders/fragment.glsl"));
    auto defaultShader = m_vengine->renderer->shaders->get("default");
    if (!defaultShader) {
        spdlog::error(defaultShader.error().message);
        return;
    }

    // create materials (textures + shaders or just shaders)
    m_vengine->renderer->materials->add("default", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto defaultMaterial = m_vengine->renderer->materials->get("default");
    defaultMaterial->setBool("uUseTexture", false);
    defaultMaterial->setVec4("uColor", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

    auto textured = m_vengine->renderer->materials->get("default");
    textured->setBool("uUseTexture", true);
    textured->setTexture("uTexture", std::move(texture));

    m_vengine->renderer->materials->add("default2", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto textured2 = m_vengine->renderer->materials->get("default2");
    textured2->setBool("uUseTexture", true);
    textured2->setTexture("uTexture", std::move(texture2));

    // load font
    auto fonts = m_vengine->renderer->fonts->load("default", "inter_24_regular.ttf", 24);
    if (!fonts) {
        spdlog::error(fonts.error().message);
    }

    // skybox
    // order matters here! right, left, top, bottom, back, front
    m_vengine->timers->start("skybox");
    std::vector<std::string> skyboxFaces = {"resources/textures/skybox/cube_right.png", "resources/textures/skybox/cube_left.png",
                                            "resources/textures/skybox/cube_up.png",    "resources/textures/skybox/cube_down.png",
                                            "resources/textures/skybox/cube_back.png",  "resources/textures/skybox/cube_front.png"};

    if (!m_vengine->renderer->loadSkybox(skyboxFaces)) {
        spdlog::error("Failed to load skybox textures");
    }
    auto time = m_vengine->timers->stop("skybox");
    spdlog::info("Skybox loaded in {} ms", time);

    // layers
    m_testLayer = std::make_shared<TestLayer>(m_vengine);
    m_vengine->addLayer(m_testLayer);

    auto end = m_vengine->timers->stop("app_constructor");
    spdlog::info("App constructor took {} ms", end);

    // meshes
    auto cubeMesh = m_vengine->meshLoader->loadFromObj("box.obj");
    auto chairMesh = m_vengine->meshLoader->loadFromObj("chair02.obj");

    // ecs stuff
    // ground
    auto groundMesh = m_vengine->meshLoader->createPlane(500.0f, 500.0f, 1, 1);
    auto groundEntity = m_vengine->ecs->createEntity();
    m_vengine->ecs->addComponent<Vengine::MeshComponent>(groundEntity, Vengine::ComponentType::MeshBit, groundMesh);
    m_vengine->ecs->addComponent<Vengine::TransformComponent>(groundEntity, Vengine::ComponentType::TransformBit);
    m_vengine->ecs->addComponent<Vengine::RigidbodyComponent>(groundEntity, Vengine::ComponentType::RigidBodyBit);
    auto rigidBody = m_vengine->ecs->getEntityComponent<Vengine::RigidbodyComponent>(groundEntity, Vengine::ComponentType::RigidBodyBit);
    rigidBody->isStatic = true;
    m_vengine->ecs->addComponent<Vengine::MaterialComponent>(groundEntity, Vengine::ComponentType::MaterialBit, defaultMaterial);
    auto planeBounds = groundMesh->getBounds();
    m_vengine->ecs->addComponent<Vengine::ColliderComponent>(groundEntity, Vengine::ComponentType::ColliderBit, planeBounds.first,
                                                             planeBounds.second);

    // chair
    auto chairBounds = chairMesh->getBounds();
    auto chairEntity = m_vengine->ecs->createEntity();
    m_vengine->ecs->addComponent<Vengine::MeshComponent>(chairEntity, Vengine::ComponentType::MeshBit, chairMesh);
    m_vengine->ecs->addComponent<Vengine::TransformComponent>(chairEntity, Vengine::ComponentType::TransformBit);
    m_vengine->ecs->addComponent<Vengine::MaterialComponent>(chairEntity, Vengine::ComponentType::MaterialBit, textured2);
    m_vengine->ecs->addComponent<Vengine::PositionComponent>(chairEntity, Vengine::ComponentType::PositionBit);
    m_vengine->ecs->addComponent<Vengine::VelocityComponent>(chairEntity, Vengine::ComponentType::VelocityBit);
    m_vengine->ecs->addComponent<Vengine::RigidbodyComponent>(chairEntity, Vengine::ComponentType::RigidBodyBit);
    auto chairTransform =
        m_vengine->ecs->getEntityComponent<Vengine::TransformComponent>(chairEntity, Vengine::ComponentType::TransformBit);
    chairTransform->position = glm::vec3(-25.0f, 100.0f, 5.0f);
    chairTransform->scale = glm::vec3(0.15f, 0.15f, 0.15f);
    m_vengine->ecs->addComponent<Vengine::ColliderComponent>(chairEntity, Vengine::ComponentType::ColliderBit, chairBounds.first,
                                                             chairBounds.second);

    // cube
    auto cubeBounds = cubeMesh->getBounds();
    auto cubeEntity = m_vengine->ecs->createEntity();
    m_vengine->ecs->addComponent<Vengine::MeshComponent>(cubeEntity, Vengine::ComponentType::MeshBit, cubeMesh);
    m_vengine->ecs->addComponent<Vengine::TransformComponent>(cubeEntity, Vengine::ComponentType::TransformBit);
    auto boxTransform = m_vengine->ecs->getEntityComponent<Vengine::TransformComponent>(cubeEntity, Vengine::ComponentType::TransformBit);
    boxTransform->position = glm::vec3(25.0f, 120.0f, 5.0f);
    boxTransform->scale = glm::vec3(20.0f, 20.0f, 20.0f);
    m_vengine->ecs->addComponent<Vengine::MaterialComponent>(cubeEntity, Vengine::ComponentType::MaterialBit, defaultMaterial);
    m_vengine->ecs->addComponent<Vengine::RigidbodyComponent>(cubeEntity, Vengine::ComponentType::RigidBodyBit);
    auto boxRigidBody = m_vengine->ecs->getEntityComponent<Vengine::RigidbodyComponent>(cubeEntity, Vengine::ComponentType::RigidBodyBit);
    // boxRigidBody->isStatic = true;
    m_vengine->ecs->addComponent<Vengine::ColliderComponent>(cubeEntity, Vengine::ComponentType::ColliderBit, cubeBounds.first,
                                                             cubeBounds.second);

    // a grid of cubes
    // int gridWidth = 30;
    // int gridHeight = 30;
    // float spacingX = 2.4f;
    // float spacingY = 2.4f;
    // float startX = -(static_cast<float>(gridWidth) / 2.0f) * spacingX;
    // float startY = (static_cast<float>(gridHeight) / 2.0f) * spacingY;

    // for (int row = 0; row < gridHeight; ++row) {
    //     for (int col = 0; col < gridWidth; ++col) {
    //         auto entity = m_vengine->ecs->createEntity();
    //         m_vengine->ecs->addComponent<Vengine::MeshComponent>(entity, Vengine::ComponentType::MeshBit, cube);
    //         m_vengine->ecs->addComponent<Vengine::TransformComponent>(entity, Vengine::ComponentType::TransformBit);

    //         int overallIndex = row * gridWidth + col;
    //         if (overallIndex % 2 == 0) {
    //             m_vengine->ecs->addComponent<Vengine::MaterialComponent>(entity, Vengine::ComponentType::MaterialBit, textured);
    //         } else {
    //             m_vengine->ecs->addComponent<Vengine::MaterialComponent>(entity, Vengine::ComponentType::MaterialBit, textured2);
    //         }

    //         float currentX = startX + static_cast<float>(col) * spacingX;
    //         float currentY = startY - static_cast<float>(row) * spacingY;
    //         m_vengine->ecs->getEntityComponent<Vengine::TransformComponent>(entity, Vengine::ComponentType::TransformBit)->position =
    //             glm::vec3(currentX, currentY, 0.0f);
    //     }
    // }
}

void App::run() {
    auto end = m_vengine->timers->stop("vengine.start");
    spdlog::info("Full App initialization took {} ms", end);
    m_vengine->run();
}

int main() {
    App app;
    app.run();

    return 0;
}
