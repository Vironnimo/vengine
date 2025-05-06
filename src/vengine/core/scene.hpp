#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>

#include "vengine/core/cameras.hpp"
#include "vengine/ecs/entities.hpp"

namespace Vengine {

class Vengine;  

// TODO: add cameras to the scene, so we can have multiple cameras in a scene and each scene has it's own cameras
// the renderer probably would then take in the scene, which has the entities and the cameras
// and the renderer would then render the scene with the active camera of the scene

class Scene {
   public:
    Scene(std::string name) : m_name(std::move(name)) {
        m_cameras = std::make_unique<Cameras>();
    }
    virtual ~Scene() = default;

    virtual void load(Vengine& vengine) = 0;
    virtual void cleanup(Vengine& vengine) = 0;

    auto setEntities(std::shared_ptr<Entities> entities) -> void {
        m_entities = std::move(entities);
    }

    [[nodiscard]] auto getEntities() const -> std::shared_ptr<Entities> {
        return m_entities;
    }

    [[nodiscard]] auto getCameras() const -> std::shared_ptr<Cameras> {
        return m_cameras;
    }

    [[nodiscard]] auto getName() const -> const std::string& {
        return m_name;
    }

   protected:
    // TODO: change names or what?
    std::string m_name;
    std::shared_ptr<Cameras> m_cameras;
    std::shared_ptr<Entities> m_entities;
    
   private:
};

}  // namespace Vengine