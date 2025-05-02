#include "test_scene.hpp"

#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"

void TestScene::load(Vengine::Vengine& vengine) {
    spdlog::debug("Constructor TestScene");

    // skybox
    // order matters here! right, left, top, bottom, back, front
    auto skyboxRight = vengine.resourceManager->get<Vengine::Texture>("skybox_right");
    auto skyboxLeft = vengine.resourceManager->get<Vengine::Texture>("skybox_left");
    auto skyboxTop = vengine.resourceManager->get<Vengine::Texture>("skybox_top");
    auto skyboxBottom = vengine.resourceManager->get<Vengine::Texture>("skybox_bottom");
    auto skyboxBack = vengine.resourceManager->get<Vengine::Texture>("skybox_back");
    auto skyboxFront = vengine.resourceManager->get<Vengine::Texture>("skybox_front");

    std::vector<std::shared_ptr<Vengine::Texture>> skyboxTextures = {skyboxRight,  skyboxLeft, skyboxTop,
                                                                     skyboxBottom, skyboxBack, skyboxFront};
    vengine.renderer->loadSkybox(skyboxTextures);

    // create materials (textures + shaders or just shaders)
    auto texture = vengine.resourceManager->get<Vengine::Texture>("test_texture");
    auto texture2 = vengine.resourceManager->get<Vengine::Texture>("test_texture2");
    auto defaultShader = vengine.renderer->shaders->get("default");

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

    // load objects into meshes
    auto cubeMesh = vengine.meshLoader->loadFromObj("box.obj");
    auto chairMesh = vengine.meshLoader->loadFromObj("chair.obj");
    auto groundMesh = vengine.meshLoader->createPlane(500.0f, 500.0f, 1, 1);

    // ecs stuff
    // ground entity
    auto groundEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::MeshComponent>(groundEntity, groundMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(groundEntity);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(groundEntity);
    auto rigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(groundEntity);
    rigidBody->isStatic = true;
    vengine.ecs->addComponent<Vengine::MaterialComponent>(groundEntity, texturedMaterial);
    auto planeBounds = groundMesh->getBounds();
    vengine.ecs->addComponent<Vengine::ColliderComponent>(groundEntity, planeBounds.first, planeBounds.second);

    // chair entity
    auto chairBounds = chairMesh->getBounds();
    auto chairEntity = vengine.ecs->createEntity();
    // vengine.ecs->addComponent<Vengine::TagComponent>(chairEntity, "chair");
    vengine.ecs->addComponent<Vengine::MeshComponent>(chairEntity, chairMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(chairEntity);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(chairEntity, texturedMaterial2);
    vengine.ecs->addComponent<Vengine::VelocityComponent>(chairEntity);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(chairEntity);
    auto chairTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(chairEntity);
    chairTransform->position = glm::vec3(-25.0f, 100.0f, 5.0f);
    chairTransform->scale = glm::vec3(0.15f, 0.15f, 0.15f);
    vengine.ecs->addComponent<Vengine::ColliderComponent>(chairEntity, chairBounds.first, chairBounds.second);

    // cube entity
    auto cubeBounds = cubeMesh->getBounds();
    auto cubeEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(cubeEntity, "cube");
    vengine.ecs->addComponent<Vengine::MeshComponent>(cubeEntity, cubeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(cubeEntity);
    auto boxTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(cubeEntity);
    boxTransform->position = glm::vec3(25.0f, 120.0f, 5.0f);
    boxTransform->scale = glm::vec3(20.0f, 20.0f, 20.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(cubeEntity, coloredMaterial);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(cubeEntity);
    auto boxRigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(cubeEntity);
    // boxRigidBody->isStatic = true;
    vengine.ecs->addComponent<Vengine::ColliderComponent>(cubeEntity, cubeBounds.first, cubeBounds.second);

    // test entity class
    auto testEntity = vengine.ecs->getEntityByTag("chair");
    if (testEntity.getId() != 0) {
        spdlog::warn("Test entity ID: {}", testEntity.getId());
        auto comp = testEntity.getComponent<Vengine::MeshComponent>();
        spdlog::warn("Test entity component stuff: {}", comp->mesh->getVertexCount());
        auto transform = testEntity.getComponent<Vengine::TransformComponent>();
        spdlog::warn("Test entity transform stuff: {} {} {}", transform->position.x, transform->position.y, transform->position.z);
        auto result = testEntity.hasComponent<Vengine::PositionComponent>();
        spdlog::warn("Test entity has position component: {}", result);
        testEntity.addComponent<Vengine::PositionComponent>();
        auto result2 = testEntity.hasComponent<Vengine::PositionComponent>();
        spdlog::warn("Test entity has position component2: {}", result2);
        auto position = testEntity.getComponent<Vengine::PositionComponent>();
        spdlog::warn("Test entity position stuff: {} {}", position->x, position->y);
        testEntity.removeComponent<Vengine::PositionComponent>();
        auto position2 = testEntity.getComponent<Vengine::PositionComponent>();
        if (position2) {
            spdlog::warn("Test entity position stuff: {} {}", position2->x, position2->y);
        } else {
            spdlog::warn("Test entity position component removed successfully");
        }
    }
}

void TestScene::cleanup(Vengine::Vengine& vengine) {
    spdlog::info("Cleaning up TestScene: {}", m_name);

    vengine.renderer->unloadSkybox();
}
