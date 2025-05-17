#include "test_scene2.hpp"

#include "vengine/ecs/components.hpp"
#include "vengine/utils/utils.hpp"

TestScene2::~TestScene2() {
    spdlog::debug("Destructor TestScene2");
}

void TestScene2::load(Vengine::Vengine& vengine) {
    spdlog::debug("Constructor TestScene2");
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
    camTransform->setPosition(0.0f, -82.0f, 50.0f);
    // don't forget the aspect ratio
    auto camComp = vengine.ecs->getEntityComponent<Vengine::CameraComponent>(mainCameraEntity);
    camComp->aspectRatio = static_cast<float>(vengine.window->getWidth()) / static_cast<float>(vengine.window->getHeight());

    auto cubeMesh = vengine.resourceManager->get<Vengine::Mesh>("cube");

    auto texture = vengine.resourceManager->get<Vengine::Texture>("test_texture");
    auto texture2 = vengine.resourceManager->get<Vengine::Texture>("test_texture2");
    auto defaultShader = vengine.resourceManager->get<Vengine::Shader>("default");

    vengine.renderer->materials->add("default", std::make_shared<Vengine::Material>(defaultShader));
    auto texturedMaterial = vengine.renderer->materials->get("default");
    texturedMaterial->setBool("uUseTexture", true);
    texturedMaterial->setTexture("uTexture", std::move(texture));

    vengine.renderer->materials->add("default2", std::make_shared<Vengine::Material>(defaultShader));
    auto texturedMaterial2 = vengine.renderer->materials->get("default2");
    texturedMaterial2->setBool("uUseTexture", true);
    texturedMaterial2->setTexture("uTexture", std::move(texture2));

    // ground2 entity
    // auto groundBounds = cubeMesh->getBounds();
    auto groundEntity2 = vengine.ecs->createEntity();
    vengine.ecs->addComponent<Vengine::TagComponent>(groundEntity2, "cube3");
    vengine.ecs->addComponent<Vengine::MeshComponent>(groundEntity2, cubeMesh);
    vengine.ecs->addComponent<Vengine::TransformComponent>(groundEntity2);
    auto boxTransform3 = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(groundEntity2);
    boxTransform3->setPosition(0.0f, -100.0f, 0.0f);
    boxTransform3->setScale(200.0f, 0.1f, 200.0f);
    vengine.ecs->addComponent<Vengine::MaterialComponent>(groundEntity2, texturedMaterial);
    vengine.ecs->addComponent<Vengine::PhysicsComponent>(groundEntity2);
    auto jolt = vengine.ecs->getEntityComponent<Vengine::PhysicsComponent>(groundEntity2);
    jolt->isStatic = true;
    // jolt->restitution = 0.2f;
    // jolt->friction = 1.0f;

    // a grid of cubes
    int gridWidth = 50;
    int gridHeight = 50;
    float spacingX = 2.8f;
    float spacingY = 2.8f;
    float startX = -(static_cast<float>(gridWidth) / 2.0f) * spacingX;
    float startY = (static_cast<float>(gridHeight) / 2.0f) * spacingY;

    for (int row = 0; row < gridHeight; ++row) {
        for (int col = 0; col < gridWidth; ++col) {
            auto entity = vengine.ecs->createEntity();
            vengine.ecs->addComponent<Vengine::MeshComponent>(entity, cubeMesh);
            vengine.ecs->addComponent<Vengine::TransformComponent>(entity);
            auto meshTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(entity);
            meshTransform->setScale(Vengine::Utils::getRandomFloat(0.7f, 1.3f));

            int overallIndex = row * gridWidth + col;
            if (overallIndex % 2 == 0) {
                vengine.ecs->addComponent<Vengine::MaterialComponent>(entity, texturedMaterial);
            } else {
                vengine.ecs->addComponent<Vengine::MaterialComponent>(entity, texturedMaterial2);
            }

            vengine.ecs->addComponent<Vengine::PhysicsComponent>(entity);
            auto jolt = vengine.ecs->getEntityComponent<Vengine::PhysicsComponent>(entity);
            // jolt->restitution = 0.8f;
            // jolt->friction = 0.2f;

            float currentX = startX + static_cast<float>(col) * spacingX;
            float currentY = startY - static_cast<float>(row) * spacingY;
            vengine.ecs->getEntityComponent<Vengine::TransformComponent>(entity)->setPosition(currentX, currentY, 0.0f);
        }
    }
}

void TestScene2::cleanup(Vengine::Vengine& vengine) {
    (void)vengine;
}
