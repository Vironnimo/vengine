#include "editor_scene.hpp"

namespace Veditor {

void EditorScene::load(Vengine::Vengine& vengine) {
    // cam
    auto camScript = vengine.resourceManager->get<Vengine::Script>("camera");
    auto editorCamId = vengine.ecs->createEntity();
    auto editorCam = vengine.ecs->getEntity(editorCamId);
    editorCam.addComponent<Vengine::TagComponent>("editor.camera");
    editorCam.addComponent<Vengine::TransformComponent>();
    editorCam.addComponent<Vengine::CameraComponent>();
    editorCam.addComponent<Vengine::ScriptComponent>(camScript);
    vengine.scenes->getCurrentScene()->getCameras()->add(editorCamId);
    vengine.scenes->getCurrentScene()->getCameras()->setActive(editorCamId);

    auto camTransform = vengine.ecs->getEntityComponent<Vengine::TransformComponent>(editorCamId);
    camTransform->setPosition(0.0f, 5.0f, 20.0f);
    // don't forget the aspect ratio
    auto camComp = vengine.ecs->getEntityComponent<Vengine::CameraComponent>(editorCamId);
    camComp->aspectRatio = static_cast<float>(vengine.window->getWidth()) / static_cast<float>(vengine.window->getHeight());

    // render a simple cube without everything, no texture, no material
    auto cubeModel = vengine.resourceManager->get<Vengine::Model>("cube");
    auto cubeId = vengine.ecs->createEntity();
    auto cube = vengine.ecs->getEntity(cubeId);
    cube.addComponent<Vengine::TagComponent>("cube_no_texture");
    cube.addComponent<Vengine::TransformComponent>();
    cube.addComponent<Vengine::ModelComponent>(cubeModel);

    // render cube with texture
    auto cubeTextureModel = vengine.resourceManager->get<Vengine::Model>("cube");
    auto cubeTextureId = vengine.ecs->createEntity();
    auto cubeTexture = vengine.ecs->getEntity(cubeTextureId);
    cubeTexture.addComponent<Vengine::TagComponent>("cube_with_texture");
    cubeTexture.addComponent<Vengine::TransformComponent>();
    cubeTexture.addComponent<Vengine::ModelComponent>(cubeTextureModel);

    // render a 3d model with submeshes

    // render 3d model with submeshes and put 1 texture on the model

    // render 3d model with submeshes and texture each submesh differently

    // render 3d model with integrated textures

    // more ??
}

void EditorScene::cleanup(Vengine::Vengine& vengine) {
}

}  // namespace Veditor