#pragma once

#include <vengine/core/module.hpp>

namespace Veditor {

class EditorModule : public Vengine::Module {
   public:
    EditorModule() = default;
    ~EditorModule() override = default;

    void onAttach(Vengine::Vengine& vengine) override;
    void onUpdate(Vengine::Vengine& vengine, float deltaTime) override;
    void onDetach(Vengine::Vengine& vengine) override;
};

}  // namespace Veditor