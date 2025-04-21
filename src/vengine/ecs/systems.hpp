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
            auto position = entities->getEntityComponent<PositionComponent>(entity, ComponentType::PositionBit);
            auto velocity = entities->getEntityComponent<VelocityComponent>(entity, ComponentType::VelocityBit);
            position->x += velocity->dx * deltaTime;
            position->y += velocity->dy * deltaTime;

            // reset velocity
            velocity->dx = 0.0f;
            velocity->dy = 0.0f;
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
        if (!m_camera) {
            spdlog::error("RenderSystem cannot update without a camera.");
            return;
        }

        // camera matrices
        glm::mat4 viewMatrix = m_camera->getViewMatrix();
        glm::mat4 projectionMatrix = m_camera->getProjectionMatrix();

        auto list =
            entities->getEntitiesWith(ComponentType::TransformBit, ComponentType::MeshBit, ComponentType::MaterialBit);

        for (auto entity : list) {
            auto transformComp = entities->getEntityComponent<TransformComponent>(entity, ComponentType::TransformBit);
            auto meshComp = entities->getEntityComponent<MeshComponent>(entity, ComponentType::MeshBit);
            auto materialComp = entities->getEntityComponent<MaterialComponent>(entity, ComponentType::MaterialBit);

            if (transformComp && meshComp && meshComp->mesh && materialComp && materialComp->material) {
                materialComp->material->bind();

                auto shader = materialComp->material->getShader();
                if (!shader) {
                    spdlog::warn("Material has no shader, skipping entity {}", entity);
                    continue;  
                }

                transformComp->updateMatrix();                                   

                shader->setUniformMat4("uView", viewMatrix);
                shader->setUniformMat4("uProjection", projectionMatrix);
                shader->setUniformMat4("uTransform", transformComp->transform);  

                meshComp->mesh->draw();
            } else {
                spdlog::warn("Entity {} is missing required components for rendering", entity);
            }
        }
    }

   private:
    std::shared_ptr<Camera> m_camera;
};

}  // namespace Vengine