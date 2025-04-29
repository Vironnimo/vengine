#include "test_scene2.hpp"

void TestScene2::load(Vengine::Vengine& vengine) {
    auto cubeMesh = vengine.meshLoader->loadFromObj("box.obj");

    auto texture = vengine.resourceManager->get<Vengine::Texture>("test_texture");
    auto texture2 = vengine.resourceManager->get<Vengine::Texture>("test_texture2");
    auto defaultShader = vengine.renderer->shaders->get("default");

    vengine.renderer->materials->add("default", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto texturedMaterial = vengine.renderer->materials->get("default");
    texturedMaterial->setBool("uUseTexture", true);
    texturedMaterial->setTexture("uTexture", std::move(texture));

    vengine.renderer->materials->add("default2", std::make_shared<Vengine::Material>(defaultShader.value()));
    auto texturedMaterial2 = vengine.renderer->materials->get("default2");
    texturedMaterial2->setBool("uUseTexture", true);
    texturedMaterial2->setTexture("uTexture", std::move(texture2));

    // a grid of cubes
    int gridWidth = 30;
    int gridHeight = 30;
    float spacingX = 2.4f;
    float spacingY = 2.4f;
    float startX = -(static_cast<float>(gridWidth) / 2.0f) * spacingX;
    float startY = (static_cast<float>(gridHeight) / 2.0f) * spacingY;

    for (int row = 0; row < gridHeight; ++row) {
        for (int col = 0; col < gridWidth; ++col) {
            auto entity = vengine.ecs->createEntity();
            vengine.ecs->addComponent<Vengine::MeshComponent>(entity, Vengine::ComponentType::MeshBit, cubeMesh);
            vengine.ecs->addComponent<Vengine::TransformComponent>(entity, Vengine::ComponentType::TransformBit);

            int overallIndex = row * gridWidth + col;
            if (overallIndex % 2 == 0) {
                vengine.ecs->addComponent<Vengine::MaterialComponent>(entity, Vengine::ComponentType::MaterialBit, texturedMaterial);
            } else {
                vengine.ecs->addComponent<Vengine::MaterialComponent>(entity, Vengine::ComponentType::MaterialBit, texturedMaterial2);
            }

            float currentX = startX + static_cast<float>(col) * spacingX;
            float currentY = startY - static_cast<float>(row) * spacingY;
            vengine.ecs->getEntityComponent<Vengine::TransformComponent>(entity, Vengine::ComponentType::TransformBit)->position =
                glm::vec3(currentX, currentY, 0.0f);
        }
    }
}

void TestScene2::cleanup(Vengine::Vengine& vengine) {
}
