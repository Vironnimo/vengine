#pragma once

namespace Vengine {

class Layer {
public:
    virtual ~Layer() = default;
    
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onAttach() {}
    virtual void onDetach() {}
};

} // namespace Vengine