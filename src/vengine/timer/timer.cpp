#include "timer.hpp"

std::chrono::high_resolution_clock::time_point Timer::m_lastFrameTime = std::chrono::high_resolution_clock::now();
float Timer::m_deltaTime = 0.0f;

float Timer::getDeltaTime() { return m_deltaTime; }

void Timer::updateDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - m_lastFrameTime;
    m_deltaTime = elapsed.count();
    m_lastFrameTime = currentTime;
}