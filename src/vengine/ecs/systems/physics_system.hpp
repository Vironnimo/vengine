#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>

#include "vengine/ecs/base_system.hpp"
#include "vengine/ecs/entities.hpp"

namespace Vengine {

class PhysicsSystem : public BaseSystem {
   public:
    PhysicsSystem();
    ~PhysicsSystem() override;

    void update(std::shared_ptr<Entities> entities, float deltaTime) override;
    void removeBody(EntityId entityId, const std::shared_ptr<Entities>& entities);

   private:
    JPH::PhysicsSystem m_physicsSystem;
    JPH::TempAllocatorImpl* m_tempAllocator = nullptr;
    JPH::JobSystemThreadPool* m_jobSystem = nullptr;
    bool m_initialized = false;

    void initializeJolt();
    void createBodyForEntity(EntityId entityId, const std::shared_ptr<Entities>& entities);
};

}  // namespace Vengine