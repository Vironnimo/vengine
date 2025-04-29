#pragma once

#include "vengine/vengine.hpp"

class TestScene2 : public Vengine::Scene {
   public:
    TestScene2(std::string name) : Scene(std::move(name)) {}

    void load(Vengine::Vengine& vengine) override;
    void cleanup(Vengine::Vengine& vengine) override;

   private:
    std::shared_ptr<Vengine::TextObject> m_textObject;
};