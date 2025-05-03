#include "scenes.hpp"

#include "vengine/vengine.hpp"

namespace Vengine {

void Scenes::add(const std::string& name, std::shared_ptr<Scene> scene) {
    m_scenes[name] = std::move(scene);
    // m_scenes[name]->setEntities(std::move(entities));
}

void Scenes::switchTo(const std::string& name, Vengine& vengine) {
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        if (m_currentScene == it->second) {
            spdlog::warn("Scene '{}' is already active", name);
            return;
        }

        // weird here, probably just it->second->setEntities(vengine.ecs->getActiveEntities()); is enough?
        if (m_currentScene) {
            m_currentScene->cleanup(vengine);
            // clear all entitites except persistent ones
            m_currentScene->getEntities()->removeNonPersistentEntities();
            it->second->setEntities(m_currentScene->getEntities());
        } else {
            it->second->setEntities(vengine.ecs->getActiveEntities());
        }

        m_currentScene = it->second;

        m_currentScene->load(vengine);
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