#include "scenes.hpp"

#include "vengine/vengine.hpp"

namespace Vengine {

void Scenes::add(const std::string& name, std::shared_ptr<Scene> scene) {
    m_scenes[name] = std::move(scene);
    // m_scenes[name]->setEntities(std::move(entities));
}

void Scenes::load(const std::string& name, Vengine& vengine) {
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        if (m_currentScene == it->second) {
            spdlog::warn("Scene '{}' is already active", name);
            return;
        }

        if (m_currentScene) {
            m_currentScene->cleanup(vengine);
            // clear all entitites except persistent ones (maybe player or something)
            m_currentScene->getEntities()->removeNonPersistentEntities();
            it->second->setEntities(m_currentScene->getEntities());
        } else {
            it->second->setEntities(vengine.ecs->getActiveEntities());
        }
        m_currentScene = it->second;
        m_currentScene->load(vengine);

        // if scene has no active camera, create a default one
        if (m_currentScene->getCameras()->getActive() == 0) {
            auto camera = vengine.ecs->createEntity();
            vengine.ecs->addComponent<TagComponent>(camera, "DefaultCamera");
            vengine.ecs->addComponent<TransformComponent>(camera);
            vengine.ecs->addComponent<CameraComponent>(camera);

            m_currentScene->getCameras()->add(camera);
            m_currentScene->getCameras()->setActive(camera);

            auto camComp = vengine.ecs->getEntityComponent<CameraComponent>(camera);
            camComp->aspectRatio = static_cast<float>(vengine.window->getWidth()) / static_cast<float>(vengine.window->getHeight());
        } else {
            // set the active camera to the first one in the list
            // m_currentScene->getCameras()->setActive(m_currentScene->getCameras());
        }
    } else {
        spdlog::error("Scene '{}' not found", name);
    }
}

void Scenes::remove(std::string name) {
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        m_scenes.erase(it);
    } else {
        spdlog::error("Scene '{}' not found", name);
    }
}

}  // namespace Vengine