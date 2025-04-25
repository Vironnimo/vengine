#include "scenes.hpp"

namespace Vengine {

void Scenes::add(const std::string& name, std::shared_ptr<Scene> scene, std::shared_ptr<Entities> entities) {
    m_scenes[name] = std::move(scene);
    m_scenes[name]->setEntities(std::move(entities));
}

void Scenes::switchTo(std::shared_ptr<Scene> scene) {
    // if (m_currentScene) {
    //     m_currentScene->cleanup();
    // }
    m_currentScene = std::move(scene);
    m_currentScene->load();
}

void Scenes::switchTo(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        switchTo(it->second);
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