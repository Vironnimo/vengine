#include "jolt_physics_system.hpp"

#include <spdlog/spdlog.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace Vengine {

namespace {

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
   public:
    BroadPhaseLayerInterfaceImpl() {
        m_objectToBroadPhase[0] = JPH::BroadPhaseLayer(0);
    }
    auto GetNumBroadPhaseLayers() const -> uint32_t override {
        return 1;
    }
    auto GetBroadPhaseLayer(JPH::ObjectLayer) const -> JPH::BroadPhaseLayer override {
        return JPH::BroadPhaseLayer(0);
    }
    auto GetBroadPhaseLayerName(JPH::BroadPhaseLayer) const -> const char* {
        return "Default";
    }

   private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[1];
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
   public:
    auto ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const -> bool override {
        return true;
    }
};

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
   public:
    auto ShouldCollide(JPH::ObjectLayer, JPH::ObjectLayer) const -> bool override {
        return true;
    }
};

}  // anonymous namespace

JoltPhysicsSystem::JoltPhysicsSystem() {
    spdlog::debug("Constructor JoltPhysicsSystem");

    initializeJolt();
}

JoltPhysicsSystem::~JoltPhysicsSystem() {
    spdlog::debug("Destructor JoltPhysicsSystem");
    delete m_tempAllocator;
    delete m_jobSystem;
}

void JoltPhysicsSystem::initializeJolt() {
    if (m_initialized) {
        return;
    }

    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    m_tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    m_jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4);

    static BroadPhaseLayerInterfaceImpl broadPhaseLayerInterface;
    static ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    static ObjectLayerPairFilterImpl objectLayerPairFilter;

    m_physicsSystem.Init(1024,  // max bodies
                         0,     // num body mutexes
                         1024,  // max body pairs
                         1024,  // max contact constraints
                         broadPhaseLayerInterface,
                         objectVsBroadPhaseLayerFilter,
                         objectLayerPairFilter);
    m_initialized = true;
}

void JoltPhysicsSystem::createBodyForEntity(EntityId entityId, const std::shared_ptr<Entities>& entities) {
    auto joltComp = entities->getEntityComponent<JoltPhysicsComponent>(entityId);
    auto transform = entities->getEntityComponent<TransformComponent>(entityId);
    auto meshComp = entities->getEntityComponent<MeshComponent>(entityId);

    if (!joltComp || !transform || !meshComp || !meshComp->mesh) {
        return;
    }

    auto [meshMin, meshMax] = meshComp->mesh->getBounds();
    glm::vec3 scale = transform->getScale();
    glm::vec3 halfExtent = ((meshMax - meshMin) * 0.5f) * scale;

    JPH::BoxShapeSettings shapeSettings(JPH::Vec3(halfExtent.x, halfExtent.y, halfExtent.z));
    auto shapeResult = shapeSettings.Create();
    if (shapeResult.HasError()) {
        spdlog::error("Jolt: Failed to create shape: {}", shapeResult.GetError().c_str());
        return;
    }
    JPH::ShapeRefC shape = shapeResult.Get();

    glm::vec3 pos = transform->getPosition();
    glm::vec3 meshCenter = (meshMin + meshMax) * 0.5f;
    JPH::RVec3 joltPos(pos.x + meshCenter.x * scale.x, pos.y + meshCenter.y * scale.y, pos.z + meshCenter.z * scale.z);

    JPH::BodyCreationSettings bodySettings(shape,
                                           joltPos,
                                           JPH::Quat::sIdentity(),
                                           joltComp->isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
                                           0  // object layer
    );

    // for bounciness, 0.0 to 1.0
    bodySettings.mRestitution = 0.8f;
    // also helps with bounciness
    bodySettings.mFriction = 0.2f;

    JPH::Body* body = m_physicsSystem.GetBodyInterface().CreateBody(bodySettings);
    m_physicsSystem.GetBodyInterface().AddBody(body->GetID(), JPH::EActivation::Activate);

    joltComp->bodyId = body->GetID();
    joltComp->initialized = true;
}

void JoltPhysicsSystem::update(std::shared_ptr<Entities> entities, float deltaTime) {
    if (!m_initialized) {
        return;
    }

    auto list = entities->getEntitiesWith<JoltPhysicsComponent, TransformComponent, MeshComponent>();

    // create jolt bodies
    for (auto entityId : list) {
        auto joltComp = entities->getEntityComponent<JoltPhysicsComponent>(entityId);
        if (joltComp && !joltComp->initialized) {
            createBodyForEntity(entityId, entities);
        }
    }

    auto& bodyInterface = m_physicsSystem.GetBodyInterface();

    // apply velocity from component, which is just changed by the user
    for (auto entityId : list) {
        auto joltComp = entities->getEntityComponent<JoltPhysicsComponent>(entityId);
        auto velocityComp = entities->getEntityComponent<VelocityComponent>(entityId);
        if (joltComp && joltComp->initialized && velocityComp && !joltComp->isStatic) {
            glm::vec3 velocity = velocityComp->velocity;
            // Only apply if non-zero (optional)
            if (velocity != glm::vec3(0.0f)) {
                auto currentVel = bodyInterface.GetLinearVelocity(joltComp->bodyId);
                bodyInterface.SetLinearVelocity(joltComp->bodyId,
                                                JPH::Vec3(currentVel.GetX() + velocity.x,
                                                          currentVel.GetY() + velocity.y,
                                                          currentVel.GetZ() + velocity.z));
                velocityComp->velocity = glm::vec3(0.0f);
            }
        }
    }

    m_physicsSystem.Update(deltaTime, 1, m_tempAllocator, m_jobSystem);

    // sync back to transform component
    for (auto entityId : list) {
        auto joltComp = entities->getEntityComponent<JoltPhysicsComponent>(entityId);
        auto transform = entities->getEntityComponent<TransformComponent>(entityId);
        if (joltComp && transform && joltComp->initialized) {
            JPH::RVec3 pos = bodyInterface.GetPosition(joltComp->bodyId);
            transform->setPosition(static_cast<float>(pos.GetX()),
                                   static_cast<float>(pos.GetY()),
                                   static_cast<float>(pos.GetZ()));
            // TODO: i guess rotation aswell?
        }
    }
}

}  // namespace Vengine