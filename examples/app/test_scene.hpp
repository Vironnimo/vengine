#pragma once

#include "vengine/core/scene.hpp"

class TestScene : public Vengine::Scene {
    public:
        TestScene(std::string name) : Scene(std::move(name)) {
            spdlog::debug("Constructor TestScene");
        }
        ~TestScene() override;
        void load(Vengine::Vengine& vengine) override; 
        void cleanup(Vengine::Vengine& vengine) override;
};