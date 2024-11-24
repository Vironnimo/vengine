#pragma once

namespace Vengine {

enum ComponentType {
    Position = 1 << 0,
    Velocity = 1 << 1,
};

// Base class for all components
struct BaseComponent {
    virtual ~BaseComponent() = default;
};

// Specific components
struct PositionComponent : public BaseComponent {
    float x = 0.0f;
    float y = 0.0f;
};

struct VelocityComponent : public BaseComponent {
    float dy = 1.0f;
    float dx = 1.0f;
};

} // Vengine