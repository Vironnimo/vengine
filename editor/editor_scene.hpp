#pragma once

#include <vengine/vengine.hpp>

namespace Veditor {

class EditorScene : public Vengine::Scene {
   public:
    EditorScene(const std::string& name) : Vengine::Scene(name) {
    }

    void load(Vengine::Vengine& vengine) override;
    void cleanup(Vengine::Vengine& vengine) override;
};

}  // namespace Veditor