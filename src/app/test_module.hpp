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
    int m_testFps = 0;
    int m_testFpsCount = 0;
    float m_testFpsTimer = 0.0f;

    float m_fpsUpdateTimer = 0.0f;
};
