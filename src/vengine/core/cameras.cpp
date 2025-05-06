#include "cameras.hpp"
#include "vengine/core/event_system.hpp"

namespace Vengine {
Cameras::Cameras() {
}

Cameras::~Cameras() {
}

void Cameras::add(EntityId camera) {
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
    if (m_cameras.find(camera) == m_cameras.end()) {
        return;
    }

    g_eventSystem.publish(CameraChangedEvent{camera});
    m_activeCamera = camera;  
}

auto Cameras::getActive() const -> EntityId {
    return m_activeCamera;
}

}  // namespace Vengine