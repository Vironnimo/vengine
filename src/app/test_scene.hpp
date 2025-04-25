#pragma once

#include "vengine/core/scene.hpp"

class TestScene : public Vengine::Scene {
    public:
        TestScene(std::string name) : Scene(std::move(name)) {}
        void load() override; 
        void cleanup() override;
};