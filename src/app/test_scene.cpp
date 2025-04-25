#include "test_scene.hpp"

void TestScene::load() {
    spdlog::info("Loading TestScene: {}", m_name);
}

void TestScene::cleanup() {
    spdlog::info("Cleaning up TestScene: {}", m_name);
}
