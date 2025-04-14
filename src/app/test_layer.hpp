#pragma once

#include <memory>

#include "vengine/vengine.hpp"
#include "vengine/core/layer.hpp"
#include "vengine/renderer/renderer.hpp"

class TestLayer : public Vengine::Layer {
   public:
    TestLayer(const std::shared_ptr<Vengine::Vengine>& vengine);

    void onUpdate(float deltaTime) override;

   private:
    std::weak_ptr<Vengine::Vengine> m_vengine;
    float m_fpsUpdateTimer = 0.0f;
    std::shared_ptr<Vengine::TextObject> m_textObject;
};
