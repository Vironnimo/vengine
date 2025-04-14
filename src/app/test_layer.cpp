#include "test_layer.hpp"

#include <memory>

#include "vengine/vengine.hpp"

TestLayer::TestLayer(const std::shared_ptr<Vengine::Vengine>& vengine) : m_vengine(vengine) {
    auto venginePtr = m_vengine.lock();
    if (!venginePtr) {
        spdlog::error("TestLayer: Vengine pointer is invalid");
        return;
    }

    m_textObject = std::make_shared<Vengine::TextObject>();
    m_textObject->text = "FPS: 0";
    m_textObject->font = venginePtr->renderer->fonts->get("default").value();
    m_textObject->x = 25.0f;
    m_textObject->y = 25.0f;
    m_textObject->scale = 1.0f;
    m_textObject->color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    venginePtr->renderer->addTextObject(m_textObject);
}

void TestLayer::onUpdate(float deltaTime) {
    auto venginePtr = m_vengine.lock();
    if (!venginePtr) {
        return;
    }

    // fps
    m_fpsUpdateTimer += deltaTime;
    if (m_fpsUpdateTimer >= 0.2f) {
        m_fpsUpdateTimer = 0.0f;
        int fps = static_cast<int>(1.0f / deltaTime);
        m_textObject->text = "FPS: " + std::to_string(fps);
    }
}