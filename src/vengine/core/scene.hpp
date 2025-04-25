#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>

#include "vengine/ecs/entities.hpp"

namespace Vengine {

// in vengine class we have the ecs and the scenemanager. on creating a scene first we create a entities list,
// which we save in the ecs as current entities. then we create the scene in the scenemanager
// and give it that entities list. that one becomes the current scene. the ecs then only processes the
// current entitites list.
// on scene switch: ??
// for now we leave the systems as they are, just that they now use the current entities list.

class Vengine;  

class Scene {
   public:
    Scene(std::string name) : m_name(std::move(name)) {
    }
    virtual ~Scene() = default;

    virtual void load(Vengine& vengine) = 0;
    virtual void cleanup() = 0;

    auto setEntities(std::shared_ptr<Entities> entities) -> void {
        m_entities = std::move(entities);
    }

    [[nodiscard]] auto getEntities() const -> std::shared_ptr<Entities> {
        return m_entities;
    }

    [[nodiscard]] auto getName() const -> const std::string& {
        return m_name;
    }

   protected:
    std::string m_name;
    
   private:
    std::shared_ptr<Entities> m_entities;
};

}  // namespace Vengine