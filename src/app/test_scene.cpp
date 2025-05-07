#include "test_scene.hpp"

#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"

void TestScene::load(Vengine::Vengine& vengine) {
    spdlog::debug("Constructor TestScene");

    // cam
    Vengine::EntityId mainCameraEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(mainCameraEntity, "MainCamera");
    vengine.ecs->addComponent<Vengine::TransformComponent>(mainCameraEntity);
    vengine.ecs->addComponent<Vengine::CameraComponent>(mainCameraEntity);
    vengine.scenes->getCurrentScene()->getCameras()->add(mainCameraEntity);
    vengine.scenes->getCurrentScene()->getCameras()->setActive(mainCameraEntity);

    auto camTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(mainCameraEntity);
    camTransform->setPosition(0.0f, 10.0f, 55.0f);
    // don't forget the aspect ratio
    auto camComp = vengine.ecs->getEntityComponent<Vengine::CameraComponent>(mainCameraEntity);
    camComp->aspectRatio = static_cast<float>(vengine.window->getWidth()) / static_cast<float>(vengine.window->getHeight());

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
    auto cubeMesh = vengine.resourceManager->get<Vengine::Mesh>("cube");
    auto chairMesh = vengine.resourceManager->get<Vengine::Mesh>("chair");

    // ecs stuff
    // ground entity
    // auto groundEntity = vengine.ecs->createEntity();
    // vengine.ecs->addComponent<Vengine::MeshComponent>(groundEntity, groundMesh);
    // vengine.ecs->addComponent<Vengine::TransformComponent>(groundEntity);
    // vengine.ecs->addComponent<Vengine::RigidbodyComponent>(groundEntity);
    // auto rigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(groundEntity);
    // rigidBody->isStatic = true;
    // vengine.ecs->addComponent<Vengine::MaterialComponent>(groundEntity, texturedMaterial);
    // auto planeBounds = groundMesh->getBounds();
    // vengine.ecs->addComponent<Vengine::ColliderComponent>(groundEntity, planeBounds.first, planeBounds.second);

    // chair entity
    auto chairBounds = chairMesh->getBounds();
    auto chairEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(chairEntity, "chair");
    vengine.ecs->addComponent<Vengine::MeshComponent>(chairEntity, chairMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(chairEntity);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(chairEntity, texturedMaterial2);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(chairEntity);
    vengine.ecs->addComponent<Vengine::ScriptComponent>(chairEntity, "resources/scripts/move.lua");
    auto chairTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(chairEntity);
    chairTransform->setPosition(-25.0f, 100.0f, 5.0f);
    chairTransform->setScale(0.15f, 0.15f, 0.15f);
    vengine.ecs->addComponent<Vengine::ColliderComponent>(chairEntity, chairBounds.first, chairBounds.second);

    // cube entity
    auto cubeBounds = cubeMesh->getBounds();
    auto cubeEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(cubeEntity, "cube");
    vengine.ecs->addComponent<Vengine::MeshComponent>(cubeEntity, cubeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(cubeEntity);
    auto boxTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(cubeEntity);
    boxTransform->setPosition(25.0f, 120.0f, 5.0f);
    boxTransform->setScale(20.0f, 20.0f, 20.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(cubeEntity, coloredMaterial);
    vengine.ecs->addComponent<Vengine::RigidbodyComponent>(cubeEntity);
    auto boxRigidBody = vengine.ecs->getEntityComponent<Vengine::RigidbodyComponent>(cubeEntity);
    // boxRigidBody->isStatic = true;
    vengine.ecs->addComponent<Vengine::ColliderComponent>(cubeEntity, cubeBounds.first, cubeBounds.second);

    // test entity class
    auto testEntity = vengine.ecs->getEntityByTag("chair");
    if (testEntity.getId() != 0) {
        spdlog::debug("Test entity ID: {}", testEntity.getId());
        auto comp = testEntity.getComponent<Vengine::MeshComponent>();
        spdlog::debug("Test entity component stuff: {}", comp->mesh->getVertexCount());
        auto transform = testEntity.getComponent<Vengine::TransformComponent>();
        spdlog::debug("Test entity transform stuff: {} {} {}", transform->getPositionX(), transform->getPositionY(), transform->getPositionZ());
    }
}

void TestScene::cleanup(Vengine::Vengine& vengine) {
    spdlog::info("Cleaning up TestScene: {}", m_name);

    vengine.renderer->unloadSkybox();
}
