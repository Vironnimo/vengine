#include "physics_system.hpp"

#include <spdlog/spdlog.h>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <glm/gtc/quaternion.hpp>

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
    auto GetBroadPhaseLayer(JPH::ObjectLayer layer) const -> JPH::BroadPhaseLayer override {
        return JPH::BroadPhaseLayer(0);
    }
    auto GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const -> const char* {
        return "Default";
    }

   private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[1];
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
   public:
    auto ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broadPhaseLayer) const -> bool override {
        return true;
    }
};

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
   public:
    auto ShouldCollide(JPH::ObjectLayer layer1, JPH::ObjectLayer layer2) const -> bool override {
        return true;
    }
};

}  // anonymous namespace

PhysicsSystem::PhysicsSystem() {
    // spdlog::debug("Constructor JoltPhysicsSystem");
    initializeJolt();
}

PhysicsSystem::~PhysicsSystem() {
    // spdlog::debug("Destructor JoltPhysicsSystem");
    delete m_tempAllocator;
    delete m_jobSystem;
}

void PhysicsSystem::initializeJolt() {
    if (m_initialized) {
        return;
    }

    JPH::RegisterDefaultAllocator();
    if (JPH::Factory::sInstance == nullptr) {
        JPH::Factory::sInstance = new JPH::Factory();
    }
    JPH::RegisterTypes();

    m_tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    m_jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 8);  // 8 threads for now

    static BroadPhaseLayerInterfaceImpl broadPhaseLayerInterface;
    static ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
    static ObjectLayerPairFilterImpl objectLayerPairFilter;

    // high numbers for now
    m_physicsSystem.Init(30240,  // max bodies
                         2024,   // num body mutexes
                         10240,  // max body pairs
                         10240,  // max contact constraints
                         broadPhaseLayerInterface,
                         objectVsBroadPhaseLayerFilter,
                         objectLayerPairFilter);

    m_physicsSystem.SetGravity(JPH::Vec3(0, -9.81f, 0));

    m_initialized = true;
}

void PhysicsSystem::createBodyForEntity(EntityId entityId, const std::shared_ptr<Entities>& entities) {
    auto joltComp = entities->getEntityComponent<PhysicsComponent>(entityId);
    auto transform = entities->getEntityComponent<TransformComponent>(entityId);
    auto meshComp = entities->getEntityComponent<MeshComponent>(entityId);

    if (!joltComp || !transform || !meshComp || !meshComp->mesh) {
        return;
    }

    auto [meshMin, meshMax] = meshComp->mesh->getBounds();
    glm::vec3 scale = transform->getScale();
    glm::vec3 halfExtent = ((meshMax - meshMin) * 0.5f) * scale;

    JPH::BoxShapeSettings shapeSettings(JPH::Vec3(halfExtent.x, halfExtent.y, halfExtent.z));
    // shapeSettings.mConvexRadius = 0.0f;
    auto shapeResult = shapeSettings.Create();
    if (shapeResult.HasError()) {
        spdlog::error("Jolt: Failed to create shape: {}", shapeResult.GetError().c_str());
        return;
    }
    JPH::ShapeRefC shape = shapeResult.Get();

    glm::vec3 pos = transform->getPosition();
    glm::vec3 meshCenter = (meshMin + meshMax) * 0.5f;
    JPH::RVec3 joltPos(pos.x + meshCenter.x * scale.x, pos.y + meshCenter.y * scale.y, pos.z + meshCenter.z * scale.z);
    // JPH::RVec3 joltPos(pos.x, pos.y, pos.z);

    glm::vec3 rotation = transform->getRotation();  // (pitch, yaw, roll) or (x, y, z)
    JPH::Vec3 joltRot = JPH::Vec3(rotation.x, rotation.y, rotation.z);
    JPH::Quat joltQuat = JPH::Quat::sEulerAngles(joltRot);

    JPH::BodyCreationSettings bodySettings(shape,
                                           joltPos,
                                           joltQuat,  // <-- use the actual rotation
                                           joltComp->isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
                                           0);

    bodySettings.mRestitution = joltComp->restitution;
    bodySettings.mFriction = joltComp->friction;

    JPH::Body* body = m_physicsSystem.GetBodyInterface().CreateBody(bodySettings);
    m_physicsSystem.GetBodyInterface().AddBody(body->GetID(), JPH::EActivation::Activate);

    joltComp->bodyId = body->GetID();
    joltComp->initialized = true;
}

void PhysicsSystem::update(std::shared_ptr<Entities> entities, float deltaTime) {
    if (!m_initialized) {
        return;
    }

    // litle startup delay so objects are not beinged altered during the first frame
    if (m_startupDelay > 0.0f) {
        m_startupDelay -= deltaTime;
        return;
    }

    auto list = entities->getEntitiesWith<PhysicsComponent, TransformComponent, MeshComponent>();

    // create jolt bodies
    for (auto entityId : list) {
        auto joltComp = entities->getEntityComponent<PhysicsComponent>(entityId);
        if (joltComp && !joltComp->initialized) {
            createBodyForEntity(entityId, entities);
        }
    }

    auto& bodyInterface = m_physicsSystem.GetBodyInterface();

    // apply velocity from component, which is just changed by the user
    for (auto entityId : list) {
        auto joltComp = entities->getEntityComponent<PhysicsComponent>(entityId);
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
        auto joltComp = entities->getEntityComponent<PhysicsComponent>(entityId);
        auto transform = entities->getEntityComponent<TransformComponent>(entityId);
        if (joltComp && transform && joltComp->initialized) {
            JPH::RVec3 pos = bodyInterface.GetPosition(joltComp->bodyId);
            transform->setPosition(static_cast<float>(pos.GetX()),
                                   static_cast<float>(pos.GetY()),
                                   static_cast<float>(pos.GetZ()));

            // TODO: i guess rotation aswell?
            JPH::Quat rot = bodyInterface.GetRotation(joltComp->bodyId);
            glm::quat glmRot(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
            transform->setRotation(glm::eulerAngles(glmRot));
        }
    }
}

void PhysicsSystem::removeBody(EntityId entityId, const std::shared_ptr<Entities>& entities) {
    auto joltComp = entities->getEntityComponent<PhysicsComponent>(entityId);
    if (joltComp && joltComp->initialized) {
        auto& bodyInterface = m_physicsSystem.GetBodyInterface();
        bodyInterface.RemoveBody(joltComp->bodyId);
        bodyInterface.DestroyBody(joltComp->bodyId);
        joltComp->initialized = false;
        // spdlog::info("Jolt: Removed body for entity {}", entityId);
    }
}

}  // namespace Vengine