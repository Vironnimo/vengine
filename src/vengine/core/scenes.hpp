#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "vengine/core/scene.hpp"

namespace Vengine {

class Vengine; 

class Scenes {
   public:
    Scenes();
    ~Scenes();
    void add(const std::string& name, std::shared_ptr<Scene> scene); 
    // switchTo must be called after add
    void load(const std::string& name, Vengine& vengine);
    void remove(std::string name); 

    [[nodiscard]] auto getCurrentSceneName() const -> std::string {
        return m_currentScene->getName();
    }
    [[nodiscard]] auto getCurrentScene() const -> std::shared_ptr<Scene> {
        return m_currentScene;
    }

   private:
    std::shared_ptr<Scene> m_currentScene;
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
};

}  // namespace Vengine