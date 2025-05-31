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
    vengine.renderer->loadSkybox(skyboxTextures, vengine.resourceManager->get<Vengine::Shader>("skybox"));

    // create materials (textures + shaders or just shaders)
    auto texture = vengine.resourceManager->get<Vengine::Texture>("test_texture");
    auto texture2 = vengine.resourceManager->get<Vengine::Texture>("test_texture2");
    auto aquariumTexture = vengine.resourceManager->get<Vengine::Texture>("aquariumTexture");
    auto flowerTexture = vengine.resourceManager->get<Vengine::Texture>("flowerTexture");
    auto grassTexture = vengine.resourceManager->get<Vengine::Texture>("grass");
    auto stoneTexture = vengine.resourceManager->get<Vengine::Texture>("stone");
    auto defaultShader = vengine.resourceManager->get<Vengine::Shader>("default");

    vengine.renderer->materials->add("colored", std::make_shared<Vengine::Material>(defaultShader));
    auto coloredMaterial = vengine.renderer->materials->get("colored");
    coloredMaterial->setBool("uUseTexture", false);
    coloredMaterial->setVec4("uColor", glm::vec4(0.8f, 0.5f, 0.2f, 1.0f));

    vengine.renderer->materials->add("blue", std::make_shared<Vengine::Material>(defaultShader));
    auto blueMaterial = vengine.renderer->materials->get("blue");
    blueMaterial->setBool("uUseTexture", false);
    blueMaterial->setVec4("uColor", glm::vec4(0.2f, 0.4f, 0.8f, 1.0f));

    vengine.renderer->materials->add("stone", std::make_shared<Vengine::Material>(defaultShader));
    auto stoneMaterial = vengine.renderer->materials->get("stone");
    stoneMaterial->setBool("uUseTexture", true);
    stoneMaterial->setTexture("uTexture", std::move(stoneTexture));

    vengine.renderer->materials->add("grass", std::make_shared<Vengine::Material>(defaultShader));
    auto grassMaterial = vengine.renderer->materials->get("grass");
    grassMaterial->setBool("uUseTexture", true);
    grassMaterial->setTexture("uTexture", std::move(grassTexture));

    vengine.renderer->materials->add("flower", std::make_shared<Vengine::Material>(defaultShader));
    auto flowerMaterial = vengine.renderer->materials->get("flower");
    flowerMaterial->setBool("uUseTexture", true);
    flowerMaterial->setTexture("uTexture", std::move(flowerTexture));

    vengine.renderer->materials->add("aquarium", std::make_shared<Vengine::Material>(defaultShader));
    auto aquariumMaterial = vengine.renderer->materials->get("aquarium");
    aquariumMaterial->setBool("uUseTexture", true);
    aquariumMaterial->setTexture("uTexture", std::move(aquariumTexture));

    vengine.renderer->materials->add("default", std::make_shared<Vengine::Material>(defaultShader));
    auto texturedMaterial = vengine.renderer->materials->get("default");
    texturedMaterial->setBool("uUseTexture", true);
    texturedMaterial->setTexture("uTexture", std::move(texture));

    vengine.renderer->materials->add("default2", std::make_shared<Vengine::Material>(defaultShader));
    auto texturedMaterial2 = vengine.renderer->materials->get("default2");
    texturedMaterial2->setBool("uUseTexture", true);
    texturedMaterial2->setTexture("uTexture", std::move(texture2));

    // load objects into meshes
    auto cubeMesh = vengine.resourceManager->get<Vengine::Mesh>("cube");
    auto tankMesh = vengine.resourceManager->get<Vengine::Mesh>("tank");
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

    // light entity
    auto lightEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(lightEntity, "light");
    vengine.ecs->addComponent<Vengine::TransformComponent>(lightEntity);
    vengine.ecs->addComponent<Vengine::LightComponent>(lightEntity);
    auto lightTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(lightEntity);
    // set the light source position like a sun in the sky
    lightTransform->setPosition(20.0f, 50.0f, 20.0f);
    auto lightComp = vengine.ecs->getEntityComponent<Vengine::LightComponent>(lightEntity);
    glm::vec3 sunPos = lightTransform->getPosition();
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    lightComp->direction = glm::normalize(target - sunPos);

    // table entity
    auto tableMesh = vengine.resourceManager->get<Vengine::Model>("table_____________________");
    auto tableEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(tableEntity, "table");
    vengine.ecs->addComponent<Vengine::ModelComponent>(tableEntity, tableMesh);
    auto modelComp = vengine.ecs->getEntityComponent<Vengine::ModelComponent>(tableEntity);
    // modelComp->model->setDefaultMaterial(texturedMaterial);
    vengine.ecs->addComponent<Vengine::TransformComponent>(tableEntity);
    auto transformComp = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(tableEntity);
    transformComp->setPosition(0.0f, 1.2f, 10.0f);
    transformComp->setRotation(-1.6f, 0.0f, 0.0f);

    // mercedes entity
    auto mercedesMesh = vengine.resourceManager->get<Vengine::Model>("mercedes");
    auto mercedesEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(mercedesEntity, "mercedes");
    vengine.ecs->addComponent<Vengine::ModelComponent>(mercedesEntity, mercedesMesh);
    auto mercedesmodelComp = vengine.ecs->getEntityComponent<Vengine::ModelComponent>(mercedesEntity);
    // mercedesmodelComp->model->setDefaultMaterial(texturedMaterial);
    vengine.ecs->addComponent<Vengine::TransformComponent>(mercedesEntity);
    auto mercedesTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(mercedesEntity);
    mercedesTransform->setPosition(0.0f, 0.0f, 0.0f);
    mercedesTransform->setRotation(-1.5f, 0.00f, 0.0f);
    mercedesTransform->setScale(0.11f, 0.11f, 0.11f);

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
    treeTransform->setPosition(10.0f, 0.4f, -10.0f);

    // flower entity
    auto flowerEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(flowerEntity, "flower");
    vengine.ecs->addComponent<Vengine::MeshComponent>(flowerEntity, flowerMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(flowerEntity);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(flowerEntity, flowerMaterial);
    vengine.ecs->addComponent<Vengine::PhysicsComponent>(flowerEntity);
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
    vengine.ecs->addComponent<Vengine::PhysicsComponent>(ant);
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
    vengine.ecs->addComponent<Vengine::PhysicsComponent>(aquarium);
    auto aquariumTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(aquarium);
    aquariumTransform->setPosition(0.0f, 5.0f, 0.0f);
    // aquariumTransform->setScale(100.15f, 100.15f, 100.15f);

    // tank entity
    auto tankEntity = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(tankEntity, "tank");
    vengine.ecs->addComponent<Vengine::MeshComponent>(tankEntity, tankMesh);
    vengine.ecs->addComponent<Vengine::VelocityComponent>(tankEntity);
    vengine.ecs->addComponent<Vengine::TransformComponent>(tankEntity);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(tankEntity, texturedMaterial2);
    // vengine.ecs->addComponent<Vengine::JoltPhysicsComponent>(tankEntity);
    // auto moveScript = vengine.resourceManager->get<Vengine::Script>("move");
    // vengine.ecs->addComponent<Vengine::ScriptComponent>(tankEntity, moveScript);
    auto tankTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(tankEntity);
    tankTransform->setPosition(-25.0f, 0.0f, 0.0f);

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
    boxTransform2->setPosition(0.0f, 40.0f, -5.0f);
    // boxTransform2->setScale(20.0f, 20.0f, 20.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(cubeEntity2, coloredMaterial);
    vengine.ecs->addComponent<Vengine::PhysicsComponent>(cubeEntity2);
    auto joltComp = vengine.ecs->getEntityComponent<Vengine::PhysicsComponent>(cubeEntity2);
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
    vengine.ecs->addComponent<Vengine::PhysicsComponent>(groundEntity);
    auto jolt = vengine.ecs->getEntityComponent<Vengine::PhysicsComponent>(groundEntity);
    jolt->isStatic = true;
}

void TestScene::cleanup(Vengine::Vengine& vengine) {
    // spdlog::info("Cleaning up TestScene: {}", m_name);
    vengine.renderer->unloadSkybox();
}
