#pragma once

namespace Vengine {

class Vengine;  

class Module {

public:
    virtual ~Module() = default;
    
    virtual void onUpdate(Vengine& vengine, float deltaTime) = 0;
    virtual void onAttach(Vengine& vengine) = 0;
    virtual void onDetach(Vengine& vengine) = 0;
};

} // namespace Vengine