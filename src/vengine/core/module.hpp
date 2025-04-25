#pragma once

namespace Vengine {

class Module {
public:
    virtual ~Module() = default;
    
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onAttach() {}
    virtual void onDetach() {}
};

} // namespace Vengine