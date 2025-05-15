#include "cameras.hpp"
#include "vengine/core/event_manager.hpp"

namespace Vengine {
Cameras::Cameras() {
    spdlog::debug("Constructor Cameras");
}

Cameras::~Cameras() {
    spdlog::debug("Destructor Cameras");
}

void Cameras::add(EntityId camera) {
    assert(camera != 0 && "Camera ID cannot be 0");
    if (m_cameras.find(camera) != m_cameras.end()) {
        return;
    }

    m_cameras.insert(camera);

    if (m_activeCamera == 0) {
        m_activeCamera = camera;
    }
}

void Cameras::remove(EntityId camera) {
    m_cameras.erase(camera);
}

void Cameras::setActive(EntityId camera) {
    assert(camera != 0 && "Camera ID cannot be 0");
    if (camera == m_activeCamera) {
        return;
    }
    if (m_cameras.find(camera) == m_cameras.end()) {
        spdlog::warn("Camera {} is not registered", camera);
        return;
    }

    m_activeCamera = camera;  
    g_eventManager.publish(CameraChangedEvent{camera});
}

auto Cameras::getActive() const -> EntityId {
    return m_activeCamera;
}

}  // namespace Vengine