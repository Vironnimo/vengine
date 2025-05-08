#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <memory>
#include <utility>

#include "vengine/renderer/material.hpp"
#include "vengine/core/mesh.hpp"

namespace Vengine {

struct BaseComponent {
    virtual ~BaseComponent() = default;
};

struct TagComponent : public BaseComponent {
    TagComponent(std::string tag) : tag(std::move(tag)) {
    }
    std::string tag;
};

struct TextComponent : public BaseComponent {
    std::string text;
    std::string fontId; // store id or a ref to the font?
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
    glm::vec4 color = glm::vec4(1.0f);

    TextComponent(std::string text, std::string fontId, float x, float y, float scale, glm::vec4 color)
        : text(std::move(text)), fontId(std::move(fontId)), x(x), y(y), scale(scale), color(color) {}
};

struct ScriptComponent : public BaseComponent {
    ScriptComponent(std::string scriptPath) : path(std::move(scriptPath)) {
    }
    std::string path;
};

struct MeshComponent : public BaseComponent {
    MeshComponent(std::shared_ptr<Mesh> mesh) : mesh(std::move(mesh)) {
    }

    std::shared_ptr<Mesh> mesh;
};

struct MaterialComponent : public BaseComponent {
    MaterialComponent(std::shared_ptr<Material> material) : material(std::move(material)) {
    }

    std::shared_ptr<Material> material;
};

struct PersistentComponent : public BaseComponent {
    bool persistent = true;
};

struct TransformComponent : public BaseComponent {
   public:
    bool dirty = true;

    void updateMatrix() {
        transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, scale);
    }

    [[nodiscard]] auto getPosition() const -> glm::vec3 {
        return position;
    }
    // lua helper functions
    [[nodiscard]] auto getPositionX() const -> float {
        return position.x;
    }
    [[nodiscard]] auto getPositionY() const -> float {
        return position.y;
    }
    [[nodiscard]] auto getPositionZ() const -> float {
        return position.z;
    }
    auto setPosition(glm::vec3 position) -> void {
        this->position = position;
        dirty = true;
    }
    auto setPosition(float position) -> void {
        this->position = glm::vec3(position, position, position);
        dirty = true;
    }
    auto setPosition(float x, float y, float z) -> void {
        position.x = x;
        position.y = y;
        position.z = z;
        dirty = true;
    }

    auto getRotation() -> glm::vec3 {
        return rotation;
    }
    [[nodiscard]] auto getRotationX() const -> float {
        return rotation.x;
    }
    [[nodiscard]] auto getRotationY() const -> float {
        return rotation.y;
    }
    [[nodiscard]] auto getRotationZ() const -> float {
        return rotation.z;
    }
    auto setRotation(glm::vec3 rotation) -> void {
        this->rotation = rotation;
        dirty = true;
    }
    auto setRotation(float rotation) -> void {
        this->rotation = glm::vec3(rotation, rotation, rotation);
        dirty = true;
    }
    void setRotation(float x, float y, float z) {
        rotation.x = x;
        rotation.y = y;
        rotation.z = z;
        dirty = true;
    }

    [[nodiscard]] auto getScaleX() const -> float {
        return scale.x;
    }
    [[nodiscard]] auto getScaleY() const -> float {
        return scale.y;
    }
    [[nodiscard]] auto getScaleZ() const -> float {
        return scale.z;
    }
    auto setScale(glm::vec3 scale) -> void {
        this->scale = scale;
        dirty = true;
    }
    auto setScale(float scale) -> void {
        this->scale = glm::vec3(scale, scale, scale);
        dirty = true;
    }
    void setScale(float x, float y, float z) {
        scale.x = x;
        scale.y = y;
        scale.z = z;
        dirty = true;
    }
    [[nodiscard]] auto getTransform() const -> glm::mat4 {
        return transform;
    }

   private:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::mat4 transform = glm::mat4(1.0f);
};

struct RigidbodyComponent : public BaseComponent {
    glm::vec3 velocity = glm::vec3(0.0f);
    float mass = 1.0f;
    bool useGravity = true;
    bool isGrounded = false;
    bool isStatic = false;
};

struct ColliderComponent : public BaseComponent {
    using EntityId = uint64_t;

    glm::vec3 min = glm::vec3(-0.5f);
    glm::vec3 max = glm::vec3(0.5f);

    glm::vec3 worldMin;
    glm::vec3 worldMax;

    bool colliding = false;
    EntityId collidingWith = 0;

    ColliderComponent(const glm::vec3& minBounds, const glm::vec3& maxBounds) : min(minBounds), max(maxBounds) {
    }
    ColliderComponent() = default;

    // update world bounds for collision detection
    void updateWorldBounds(const glm::mat4& transform) {
        glm::vec4 corners[8] = {transform * glm::vec4(min.x, min.y, min.z, 1.0f),
                                transform * glm::vec4(max.x, min.y, min.z, 1.0f),
                                transform * glm::vec4(min.x, max.y, min.z, 1.0f),
                                transform * glm::vec4(min.x, min.y, max.z, 1.0f),
                                transform * glm::vec4(max.x, max.y, min.z, 1.0f),
                                transform * glm::vec4(min.x, max.y, max.z, 1.0f),
                                transform * glm::vec4(max.x, min.y, max.z, 1.0f),
                                transform * glm::vec4(max.x, max.y, max.z, 1.0f)};

        worldMin = glm::vec3(corners[0]);
        worldMax = glm::vec3(corners[0]);
        for (int i = 1; i < 8; ++i) {
            worldMin = glm::min(worldMin, glm::vec3(corners[i]));
            worldMax = glm::max(worldMax, glm::vec3(corners[i]));
        }
    }
};

struct CameraComponent : public BaseComponent {
    float fov = 70.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool isActive = true;  // current main camera

    [[nodiscard]] auto getProjectionMatrix() const -> glm::mat4 {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
    [[nodiscard]] auto getViewMatrix(const std::shared_ptr<TransformComponent>& transform) const -> glm::mat4 {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), transform->getRotationX(), glm::vec3(1.0f, 0.0f, 0.0f)) *
                             glm::rotate(glm::mat4(1.0f), transform->getRotationY(), glm::vec3(0.0f, 1.0f, 0.0f)) *
                             glm::rotate(glm::mat4(1.0f), transform->getRotationZ(), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), -transform->getPosition());
        return rotation * translation;
    }

    // lua helper functions
    [[nodiscard]] auto getFov() const -> float {
        return fov;
    }
    [[nodiscard]] auto getAspectRatio() const -> float {
        return aspectRatio;
    }
    [[nodiscard]] auto getNearPlane() const -> float {
        return nearPlane;
    }
    [[nodiscard]] auto getFarPlane() const -> float {
        return farPlane;
    }
    void setFov(float fov) {
        this->fov = fov;
    }
    void setAspectRatio(float aspectRatio) {
        this->aspectRatio = aspectRatio;
    }
    void setNearPlane(float nearPlane) {
        this->nearPlane = nearPlane;
    }
    void setFarPlane(float farPlane) {
        this->farPlane = farPlane;
    }
    void setActive(bool active) {
        isActive = active;
    }
    [[nodiscard]] auto isActiveCamera() const -> bool {
        return isActive;
    }
};

}  // namespace Vengine