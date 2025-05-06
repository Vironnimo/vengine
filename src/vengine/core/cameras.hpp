#pragma once

#include <unordered_set>

#include "vengine/ecs/entity.hpp"

namespace Vengine {

// NOTE: currently this is not connected to the ecs, so we don't now what cameras still exist,
// we need to remove the cameras manually
// maybe it should be part of the scene? hmm
// like check after scene load() if the scene has a active camera, and if not, set the first one as active
// or create a new default cam and set it as active?
class Cameras {
   public:
    Cameras();
    ~Cameras();

    void add(EntityId camera);
    void remove(EntityId camera);

    void setActive(EntityId camera);
    [[nodiscard]] auto getActive() const -> EntityId;

   private:
    EntityId m_activeCamera = 0;
    std::unordered_set<EntityId> m_cameras;
};

}  // namespace Vengine