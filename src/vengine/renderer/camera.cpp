#include "camera.hpp"

#include <spdlog/spdlog.h>

namespace Vengine {

Camera::Camera(CameraSettings settings) : m_settings(settings) {
    updateProjection();
}

auto Camera::setPosition(const glm::vec3& position) -> void {
    m_position = position;
    updateView();
}

auto Camera::setRotation(const glm::vec3& rotation) -> void {
    m_rotation = rotation;
    updateView();
}

auto Camera::setAspectRatio(float aspectRatio) -> void {
    m_settings.aspectRatio = aspectRatio;
    updateProjection();
}

[[nodiscard]] auto Camera::getViewMatrix() const -> glm::mat4 {
    return m_viewMatrix;
}

[[nodiscard]] auto Camera::getProjectionMatrix() const -> glm::mat4 {
    return m_projectionMatrix;
}

[[nodiscard]] auto Camera::getFov() const -> float {
    return m_settings.fov;
}

auto Camera::setFov(float fov) -> void {
    m_settings.fov = fov;
    updateProjection();
}

auto Camera::updateView() -> void {
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
                         glm::rotate(glm::mat4(1.0f), m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
                         glm::rotate(glm::mat4(1.0f), m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_position);
    m_viewMatrix = rotation * translation;
}

auto Camera::updateProjection() -> void {
    m_projectionMatrix = glm::perspective(glm::radians(m_settings.fov), m_settings.aspectRatio, m_settings.nearPlane,
                                          m_settings.farPlane);
}

}  // namespace Vengine