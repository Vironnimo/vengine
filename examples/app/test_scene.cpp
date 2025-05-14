#include "test_scene.hpp"

#include "vengine/ecs/components.hpp"
#include "vengine/vengine.hpp"

TestScene::~TestScene() {
    spdlog::debug("Destructor TestScene");
}

void TestScene::load(Vengine::Vengine& vengine) {
    // cam
    Vengine::EntityId mainCameraEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(mainCameraEntity, "MainCamera");
    vengine.ecs->addComponent<Vengine::TransformComponent>(mainCameraEntity);
    vengine.ecs->addComponent<Vengine::CameraComponent>(mainCameraEntity);
    auto script = vengine.resourceManager->get<Vengine::Script>("camera");
    vengine.ecs->addComponent<Vengine::ScriptComponent>(mainCameraEntity, script);
    vengine.scenes->getCurrentScene()->getCameras()->add(mainCameraEntity);
    vengine.scenes->getCurrentScene()->getCameras()->setActive(mainCameraEntity);

    auto camTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(mainCameraEntity);
    camTransform->setPosition(0.0f, 5.0f, 20.0f);
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

    std::vector<std::shared_ptr<Vengine::Texture>> skyboxTextures =
        {skyboxRight, skyboxLeft, skyboxTop, skyboxBottom, skyboxBack, skyboxFront};
    vengine.renderer->loadSkybox(skyboxTextures);

    // create materials (textures + shaders or just shaders)
    auto texture = vengine.resourceManager->get<Vengine::Texture>("test_texture");
    auto texture2 = vengine.resourceManager->get<Vengine::Texture>("test_texture2");
    auto aquariumTexture = vengine.resourceManager->get<Vengine::Texture>("aquariumTexture");
    auto flowerTexture = vengine.resourceManager->get<Vengine::Texture>("flowerTexture");
    auto grassTexture = vengine.resourceManager->get<Vengine::Texture>("grass");
    auto stoneTexture = vengine.resourceManager->get<Vengine::Texture>("stone");
    auto defaultShader = vengine.renderer->shaders->get("default");

    vengine.renderer->materials->add("colored", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto coloredMaterial = vengine.renderer->materials->get("colored");
    coloredMaterial->setBool("uUseTexture", false);
    coloredMaterial->setVec4("uColor", glm::vec4(0.8f, 0.5f, 0.2f, 1.0f));

    vengine.renderer->materials->add("blue", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto blueMaterial = vengine.renderer->materials->get("blue");
    blueMaterial->setBool("uUseTexture", false);
    blueMaterial->setVec4("uColor", glm::vec4(0.2f, 0.4f, 0.8f, 1.0f));


    vengine.renderer->materials->add("stone", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto stoneMaterial = vengine.renderer->materials->get("stone");
    stoneMaterial->setBool("uUseTexture", true);
    stoneMaterial->setTexture("uTexture", std::move(stoneTexture));

    vengine.renderer->materials->add("grass", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto grassMaterial = vengine.renderer->materials->get("grass");
    grassMaterial->setBool("uUseTexture", true);
    grassMaterial->setTexture("uTexture", std::move(grassTexture));

    vengine.renderer->materials->add("flower", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto flowerMaterial = vengine.renderer->materials->get("flower");
    flowerMaterial->setBool("uUseTexture", true);
    flowerMaterial->setTexture("uTexture", std::move(flowerTexture));

    vengine.renderer->materials->add("aquarium", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto aquariumMaterial = vengine.renderer->materials->get("aquarium");
    aquariumMaterial->setBool("uUseTexture", true);
    aquariumMaterial->setTexture("uTexture", std::move(aquariumTexture));

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
    auto groundMesh = vengine.resourceManager->get<Vengine::Mesh>("plane");
    auto antMesh = vengine.resourceManager->get<Vengine::Mesh>("ant");
    auto aquariumMesh = vengine.resourceManager->get<Vengine::Mesh>("aquarium");
    auto flowerMesh = vengine.resourceManager->get<Vengine::Mesh>("flower");

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

    // tree entity
    auto treeMesh = vengine.resourceManager->get<Vengine::Mesh>("tree");
    auto treeEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(treeEntity, "tree");
    vengine.ecs->addComponent<Vengine::MeshComponent>(treeEntity, treeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(treeEntity);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(treeEntity, grassMaterial);
    auto materialComp = vengine.ecs->getEntityComponent<Vengine::MaterialComponent>(treeEntity);
    materialComp->materialsByName["Bark"] = texturedMaterial;
    auto treeTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(treeEntity);
    treeTransform->setPosition(10.0f, 0.4f, 0.0f);

    // flower entity
    auto flowerEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(flowerEntity, "flower");
    vengine.ecs->addComponent<Vengine::MeshComponent>(flowerEntity, flowerMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(flowerEntity);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(flowerEntity, flowerMaterial);
    vengine.ecs->addComponent<Vengine::JoltPhysicsComponent>(flowerEntity);
    auto flowerTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(flowerEntity);
    flowerTransform->setPosition(2.0f, 3.2f, 0.0f);
    // flowerTransform->setScale(100.15f, 100.15f, 100.15f);
    // flowerTransform->setRotation(0.0f, 0.55f, 0.0f);

    // ant entity
    auto ant = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(ant, "ant");
    vengine.ecs->addComponent<Vengine::MeshComponent>(ant, antMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(ant);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(ant, coloredMaterial);
    vengine.ecs->addComponent<Vengine::JoltPhysicsComponent>(ant);
    auto antTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(ant);
    antTransform->setRotation(0.0f, 0.55f, 0.0f);
    antTransform->setPosition(-2.0f, 5.0f, 0.0f);
    // antTransform->setScale(100.15f, 100.15f, 100.15f);

    // aquarium entity
    auto aquarium = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(aquarium, "aquarium");
    vengine.ecs->addComponent<Vengine::MeshComponent>(aquarium, aquariumMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(aquarium);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(aquarium, aquariumMaterial);
    vengine.ecs->addComponent<Vengine::JoltPhysicsComponent>(aquarium);
    auto aquariumTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(aquarium);
    aquariumTransform->setPosition(0.0f, 5.0f, 0.0f);
    // aquariumTransform->setScale(100.15f, 100.15f, 100.15f);

    // chair entity
    auto chairEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(chairEntity, "chair");
    vengine.ecs->addComponent<Vengine::MeshComponent>(chairEntity, chairMesh);
    vengine.ecs->addComponent<Vengine::VelocityComponent>(chairEntity);
    vengine.ecs->addComponent<Vengine::TransformComponent>(chairEntity);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(chairEntity, texturedMaterial2);
    vengine.ecs->addComponent<Vengine::JoltPhysicsComponent>(chairEntity);
    // auto moveScript = vengine.resourceManager->get<Vengine::Script>("move");
    // vengine.ecs->addComponent<Vengine::ScriptComponent>(chairEntity, moveScript);
    auto chairTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(chairEntity);
    chairTransform->setPosition(-25.0f, 100.0f, 5.0f);
    chairTransform->setScale(0.01f, 0.01f, 0.01f);

    // cube entity
    auto cubeEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(cubeEntity, "cube");
    vengine.ecs->addComponent<Vengine::MeshComponent>(cubeEntity, cubeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(cubeEntity);
    auto boxTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(cubeEntity);
    boxTransform->setPosition(5.0f, 10.0f, 2.0f);
    // boxTransform->setScale(20.0f, 20.0f, 20.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(cubeEntity, coloredMaterial);

    // cube entity2
    auto cubeEntity2 = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(cubeEntity2, "cube2");
    vengine.ecs->addComponent<Vengine::MeshComponent>(cubeEntity2, cubeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(cubeEntity2);
    auto boxTransform2 = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(cubeEntity2);
    boxTransform2->setPosition(0.0f, 20.0f, -5.0f);
    // boxTransform2->setScale(20.0f, 20.0f, 20.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(cubeEntity2, coloredMaterial);
    vengine.ecs->addComponent<Vengine::JoltPhysicsComponent>(cubeEntity2);
    auto joltComp = vengine.ecs->getEntityComponent<Vengine::JoltPhysicsComponent>(cubeEntity2);
    joltComp->restitution = 0.5f;
    joltComp->friction = 0.2f;

    // some other ground test
    // ground2 entity
    auto groundEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(groundEntity, "ground");
    vengine.ecs->addComponent<Vengine::MeshComponent>(groundEntity, cubeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(groundEntity);
    auto boxTransform3 = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(groundEntity);
    boxTransform3->setPosition(0.0f, -0.1f, 0.0f);
    boxTransform3->setScale(50.0f, 0.1f, 50.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(groundEntity, stoneMaterial);
    vengine.ecs->addComponent<Vengine::JoltPhysicsComponent>(groundEntity);
    auto jolt = vengine.ecs->getEntityComponent<Vengine::JoltPhysicsComponent>(groundEntity);
    jolt->isStatic = true;
}

void TestScene::cleanup(Vengine::Vengine& vengine) {
    // spdlog::info("Cleaning up TestScene: {}", m_name);
    vengine.renderer->unloadSkybox();
}
