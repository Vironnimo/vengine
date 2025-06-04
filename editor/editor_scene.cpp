#include "editor_scene.hpp"

namespace Veditor {

void EditorScene::load(Vengine::Vengine& vengine) {
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

    auto entityId = vengine.ecs->createEntity();
    auto entity = vengine.ecs->getEntity(entityId);
    vengine.ecs->addComponent<Vengine::TransformComponent>(entityId);
    // entity.addComponent<Vengine::TransformComponent>();
    auto cubeModel = vengine.resourceManager->get<Vengine::Model>("cube");
    entity.addComponent<Vengine::ModelComponent>(cubeModel);
}

void EditorScene::cleanup(Vengine::Vengine& vengine) {
}

}  // namespace Veditor