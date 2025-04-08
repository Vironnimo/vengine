#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Vengine {

struct CameraSettings {
    float fov = 45.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;   // Near clipping plane
    float farPlane = 100.0f;  // Far clipping plane
};

class Camera {
   public:
    Camera(CameraSettings settings);

    auto setPosition(const glm::vec3& position) -> void;
    auto setRotation(const glm::vec3& rotation) -> void;
    auto setAspectRatio(float aspectRatio) -> void;

    [[nodiscard]] auto getPosition() const -> glm::vec3 { return m_position; }
    [[nodiscard]] auto getRotation() const -> glm::vec3 { return m_rotation; }
    [[nodiscard]] auto getAspectRatio() const -> float { return m_settings.aspectRatio; }

    [[nodiscard]] auto getViewMatrix() const -> glm::mat4;
    [[nodiscard]] auto getProjectionMatrix() const -> glm::mat4;

   private:
    CameraSettings m_settings;

    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);  // Euler angles (pitch, yaw, roll)

    glm::mat4 m_viewMatrix = glm::mat4(1.0f);
    glm::mat4 m_projectionMatrix = glm::mat4(1.0f);

    auto updateView() -> void;
    auto updateProjection() -> void;
};

}  // namespace Vengine