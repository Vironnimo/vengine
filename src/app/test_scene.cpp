#include "test_scene.hpp"

#include "vengine/vengine.hpp"

void TestScene::load(Vengine::Vengine& vengine) {
    m_name = "TestScene"; // todo set this in the constructor or during addScene, where we have a name anyway
    spdlog::info("Loading TestScene: {}", m_name);
    // load textures
    vengine.resourceManager->load<Vengine::Texture>("test_texture", "test.jpg");
    vengine.resourceManager->load<Vengine::Texture>("test_texture2", "test2.jpg");
    auto texture = vengine.resourceManager->get<Vengine::Texture>("test_texture");
    auto texture2 = vengine.resourceManager->get<Vengine::Texture>("test_texture2");
    // load fonts
    auto fonts = vengine.renderer->fonts->load("default", "inter_24_regular.ttf", 24);
    if (!fonts) {
        spdlog::error(fonts.error().message);
    }

    // load shaders
    vengine.renderer->shaders->add(
        std::make_shared<Vengine::Shader>("default", "resources/shaders/default.vert", "resources/shaders/default.frag"));
    // vengine.renderer->shaders->add(std::make_shared<Vengine::Shader>("colored", "resources/shaders/vertex.glsl",
    // "resources/shaders/fragment.glsl"));
    auto defaultShader = vengine.renderer->shaders->get("default");
    if (!defaultShader) {
        spdlog::error(defaultShader.error().message);
        return;
    }

    // create materials (textures + shaders or just shaders)
    vengine.renderer->materials->add("colored", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto coloredMaterial = vengine.renderer->materials->get("colored");
    coloredMaterial->setBool("uUseTexture", false);
    coloredMaterial->setVec4("uColor", glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));

    vengine.renderer->materials->add("default", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto texturedMaterial = vengine.renderer->materials->get("default");
    texturedMaterial->setBool("uUseTexture", true);
    texturedMaterial->setTexture("uTexture", std::move(texture));

    vengine.renderer->materials->add("default2", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto texturedMaterial2 = vengine.renderer->materials->get("default2");
    texturedMaterial2->setBool("uUseTexture", true);
    texturedMaterial2->setTexture("uTexture", std::move(texture2));

    // skybox
    // order matters here! right, left, top, bottom, back, front
    std::vector<std::string> skyboxFaces = {"resources/textures/skybox/cube_right.png", "resources/textures/skybox/cube_left.png",
                                            "resources/textures/skybox/cube_up.png",    "resources/textures/skybox/cube_down.png",
                                            "resources/textures/skybox/cube_back.png",  "resources/textures/skybox/cube_front.png"};

    if (!vengine.renderer->loadSkybox(skyboxFaces)) {
        spdlog::error("Failed to load skybox textures");
    }

    // load objects into meshes
    auto cubeMesh = vengine.meshLoader->loadFromObj("box.obj");
    auto chairMesh = vengine.meshLoader->loadFromObj("chair02.obj");
    auto groundMesh = vengine.meshLoader->createPlane(500.0f, 500.0f, 1, 1);

    // ecs stuff
    // ground entity
    auto groundEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::MeshComponent>(groundEntity, Vengine::ComponentType::MeshBit, groundMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(groundEntity, Vengine::ComponentType::TransformBit);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(groundEntity, Vengine::ComponentType::RigidBodyBit);
    auto rigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(groundEntity, Vengine::ComponentType::RigidBodyBit);
    rigidBody->isStatic = true;
    vengine.ecs->addComponent<Vengine::MaterialComponent>(groundEntity, Vengine::ComponentType::MaterialBit, texturedMaterial);
    auto planeBounds = groundMesh->getBounds();
    vengine.ecs->addComponent<Vengine::ColliderComponent>(groundEntity, Vengine::ComponentType::ColliderBit, planeBounds.first,
                                                          planeBounds.second);

    // chair entity
    auto chairBounds = chairMesh->getBounds();
    auto chairEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::MeshComponent>(chairEntity, Vengine::ComponentType::MeshBit, chairMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(chairEntity, Vengine::ComponentType::TransformBit);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(chairEntity, Vengine::ComponentType::MaterialBit, texturedMaterial2);
    vengine.ecs->addComponent<Vengine::PositionComponent>(chairEntity, Vengine::ComponentType::PositionBit);
    vengine.ecs->addComponent<Vengine::VelocityComponent>(chairEntity, Vengine::ComponentType::VelocityBit);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(chairEntity, Vengine::ComponentType::RigidBodyBit);
    auto chairTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(chairEntity, Vengine::ComponentType::TransformBit);
    chairTransform->position = glm::vec3(-25.0f, 100.0f, 5.0f);
    chairTransform->scale = glm::vec3(0.15f, 0.15f, 0.15f);
    vengine.ecs->addComponent<Vengine::ColliderComponent>(chairEntity, Vengine::ComponentType::ColliderBit, chairBounds.first,
                                                          chairBounds.second);

    // cube entity
    auto cubeBounds = cubeMesh->getBounds();
    auto cubeEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::MeshComponent>(cubeEntity, Vengine::ComponentType::MeshBit, cubeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(cubeEntity, Vengine::ComponentType::TransformBit);
    auto boxTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(cubeEntity, Vengine::ComponentType::TransformBit);
    boxTransform->position = glm::vec3(25.0f, 120.0f, 5.0f);
    boxTransform->scale = glm::vec3(20.0f, 20.0f, 20.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(cubeEntity, Vengine::ComponentType::MaterialBit, coloredMaterial);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(cubeEntity, Vengine::ComponentType::RigidBodyBit);
    auto boxRigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(cubeEntity, Vengine::ComponentType::RigidBodyBit);
    // boxRigidBody->isStatic = true;
    vengine.ecs->addComponent<Vengine::ColliderComponent>(cubeEntity, Vengine::ComponentType::ColliderBit, cubeBounds.first,
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
    //         auto entity = vengine.ecs->createEntity();
    //         vengine.ecs->addComponent<Vengine::MeshComponent>(entity, Vengine::ComponentType::MeshBit, cube);
    //         vengine.ecs->addComponent<Vengine::TransformComponent>(entity, Vengine::ComponentType::TransformBit);

    //         int overallIndex = row * gridWidth + col;
    //         if (overallIndex % 2 == 0) {
    //             vengine.ecs->addComponent<Vengine::MaterialComponent>(entity, Vengine::ComponentType::MaterialBit, textured);
    //         } else {
    //             vengine.ecs->addComponent<Vengine::MaterialComponent>(entity, Vengine::ComponentType::MaterialBit, textured2);
    //         }

    //         float currentX = startX + static_cast<float>(col) * spacingX;
    //         float currentY = startY - static_cast<float>(row) * spacingY;
    //         vengine.ecs->getEntityComponent<Vengine::TransformComponent>(entity, Vengine::ComponentType::TransformBit)->position =
    //             glm::vec3(currentX, currentY, 0.0f);
    //     }
    // }
}

void TestScene::cleanup() {
    spdlog::info("Cleaning up TestScene: {}", m_name);
}
