#pragma once

#include <typeindex>

namespace Vengine {

struct Event {
    virtual ~Event() = default;
    virtual auto getType() const -> std::type_index = 0;
};

struct MouseMovedEvent : public Event {
    int x, y;
    int lastX, lastY;
    MouseMovedEvent(int x, int y, int lastX, int lastY) : x(x), y(y), lastX(lastX), lastY(lastY) {
    }
    auto getType() const -> std::type_index override {
        return typeid(MouseMovedEvent);
    }
};

struct KeyPressedEvent : public Event {
    int key;
    bool repeat;
    KeyPressedEvent(int key, bool repeat) : key(key), repeat(repeat) {
    }
    auto getType() const -> std::type_index override {
        return typeid(KeyPressedEvent);
    }
};

struct KeyReleasedEvent : public Event {
    int key;
    KeyReleasedEvent(int key) : key(key) {
    }
    auto getType() const -> std::type_index override {
        return typeid(KeyReleasedEvent);
    }
};

struct WindowResizeEvent : public Event {
    int width, height;
    WindowResizeEvent(int width, int height) : width(width), height(height) {
    }
    auto getType() const -> std::type_index override {
        return typeid(WindowResizeEvent);
    }
};

using EntityId = uint64_t;
struct CameraChangedEvent : public Event {
    EntityId newCamera;
    CameraChangedEvent(EntityId cam) : newCamera(cam) {
    }
    auto getType() const -> std::type_index override {
        return typeid(CameraChangedEvent);
    }
};

}  // namespace Vengine