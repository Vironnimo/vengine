#pragma once

namespace Vengine {

enum ComponentType {
    Position = 1 << 0,
    Velocity = 1 << 1,
};

struct BaseComponent {
    virtual ~BaseComponent() = default;
};

struct PositionComponent : public BaseComponent {
    float x = 0.0f;
    float y = 0.0f;
};

struct VelocityComponent : public BaseComponent {
    float dy = 0.0f;
    float dx = 0.0f;
};

} // Vengine