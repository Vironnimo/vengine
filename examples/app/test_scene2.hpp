#pragma once

#include "vengine/vengine.hpp"

class TestScene2 : public Vengine::Scene {
   public:
    TestScene2(std::string name) : Scene(std::move(name)) {}
    ~TestScene2() override;

    void load(Vengine::Vengine& vengine) override;
    void cleanup(Vengine::Vengine& vengine) override;

   private:
};