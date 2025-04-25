#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "vengine/core/scene.hpp"

namespace Vengine {

class Vengine;  // Forward declaration of Vengine class

class Scenes {
   public:
    void add(const std::string& name, std::shared_ptr<Scene> scene, std::shared_ptr<Entities> entities); 
    void switchTo(const std::string& name, Vengine& vengine);
    void remove(std::string name); 

   private:
    std::shared_ptr<Scene> m_currentScene;
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
};

}  // namespace Vengine