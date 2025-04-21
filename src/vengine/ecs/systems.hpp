#pragma once

#include <spdlog/spdlog.h>

#include "entities.hpp"
#include "components.hpp"
#include "base_system.hpp"
#include "vengine/renderer/camera.hpp"

namespace Vengine {

class MovementSystem : public BaseSystem {
   public:
    void update(std::shared_ptr<Entities> entities, float deltaTime) override {
        auto list = entities->getEntitiesWith(ComponentType::PositionBit, ComponentType::VelocityBit);
        for (auto entity : list) {
            // position before
            // spdlog::debug("Entity ID: {}", entity);
            // spdlog::debug("Position X: {}", entities->getEntityComponent<PositionComponent>(entity,
            // ComponentType::Position)->x); spdlog::debug("Position Y: {}",
            // entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position)->y);
            // spdlog::debug("Velocity X: {}", entities->getEntityComponent<VelocityComponent>(entity,
            // ComponentType::Velocity)->dx); spdlog::debug("Velocity Y: {}",
            // entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity)->dy);

            auto position = entities->getEntityComponent<PositionComponent>(entity, ComponentType::PositionBit);
            auto velocity = entities->getEntityComponent<VelocityComponent>(entity, ComponentType::VelocityBit);
            position->x += velocity->dx * deltaTime;
            position->y += velocity->dy * deltaTime;

            velocity->dx = 0.0f;  // reset velocity after applying it to position
            velocity->dy = 0.0f;  // reset velocity after applying it to position

            // spdlog::debug("Entity ID: {}", entity);
            // spdlog::debug("Position X: {}", entities->getEntityComponent<PositionComponent>(entity,
            // ComponentType::Position)->x); spdlog::debug("Position Y: {}",
            // entities->getEntityComponent<PositionComponent>(entity, ComponentType::Position)->y);
            // spdlog::debug("Velocity X: {}", entities->getEntityComponent<VelocityComponent>(entity,
            // ComponentType::Velocity)->dx); spdlog::debug("Velocity Y: {}",
            // entities->getEntityComponent<VelocityComponent>(entity, ComponentType::Velocity)->dy);
        }
    }
};

class RenderSystem : public BaseSystem {
   public:
    RenderSystem(std::shared_ptr<Camera> camera) : m_camera(std::move(camera)) {
        if (!m_camera) {
            spdlog::warn("RenderSystem created without a valid Camera!");
        }
    }

    void update(std::shared_ptr<Entities> entities, float /*deltaTime*/) override {
        // In a real scenario, set Camera uniforms once here if all materials use the same shader UBO
        // or set them after binding each material's shader if they differ.
        // Example:
        // glm::mat4 viewProjection = m_camera->getViewProjectionMatrix();
        if (!m_camera) {
            spdlog::error("RenderSystem cannot update without a camera.");
            return;
        }

        // Get camera matrices once (assuming all entities use the same camera)
        glm::mat4 viewMatrix = m_camera->getViewMatrix();
        glm::mat4 projectionMatrix = m_camera->getProjectionMatrix();

        // Query for entities with all required components
        auto list =
            entities->getEntitiesWith(ComponentType::TransformBit, ComponentType::MeshBit, ComponentType::MaterialBit);

        // spdlog::debug("Entities to render: {}", list.size());
        for (auto entity : list) {
            auto transformComp = entities->getEntityComponent<TransformComponent>(entity, ComponentType::TransformBit);
            auto meshComp = entities->getEntityComponent<MeshComponent>(entity, ComponentType::MeshBit);
            auto materialComp = entities->getEntityComponent<MaterialComponent>(entity, ComponentType::MaterialBit);

            if (transformComp && meshComp && meshComp->mesh && materialComp && materialComp->material) {
                // 1. Bind the material (this binds the shader and sets material uniforms)
                materialComp->material->bind();

                // 2. Get the shader associated with this material
                auto shader = materialComp->material->getShader();
                if (!shader) {
                    spdlog::warn("Material has no shader, skipping entity {}", entity);
                    continue;  // Skip if material has no shader
                }

                // 3. Set per-frame/per-camera uniforms (if not done globally)
                // shader->setUniformMat4("u_ViewProjection", viewProjection); // Example
                shader->setUniformMat4("uView", viewMatrix);
                shader->setUniformMat4("uProjection", projectionMatrix);

                // 4. Update and set the model matrix
                transformComp->updateMatrix();                                   // Ensure matrix is up-to-date
                shader->setUniformMat4("uTransform", transformComp->transform);  // Use a standard uniform name

                // 5. Draw the mesh
                meshComp->mesh->draw();

                // Optional: Unbind VAO here if needed, but often better done after the loop
                // meshComp->mesh->getVertexArray()->unbind();
            } else {
                spdlog::warn("Entity {} is missing required components for rendering", entity);
            }
        }
        // Optional: Unbind last VAO and Shader after the loop
    }

   private:
    std::shared_ptr<Camera> m_camera;
};

}  // namespace Vengine