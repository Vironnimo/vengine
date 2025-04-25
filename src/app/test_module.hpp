#pragma once

#include <memory>

#include "vengine/vengine.hpp"
#include "vengine/core/module.hpp"

class TestModule : public Vengine::Module {
   public:
    void onAttach(Vengine::Vengine& vengine) override;
    void onUpdate(Vengine::Vengine& vengine, float deltaTime) override;
    void onDetach(Vengine::Vengine& vengine) override;

   private:
    float m_fpsUpdateTimer = 0.0f;
    std::shared_ptr<Vengine::TextObject> m_textObject;
};
