#include "timers.hpp"

#include <chrono>
#include <spdlog/spdlog.h>

namespace Vengine {

Timers::Timers() : m_lastFrameTime(std::chrono::high_resolution_clock::now()) {
    spdlog::debug("Constructor Timer");
}

Timers::~Timers() {
    spdlog::debug("Destructor Timer");
}

auto Timers::start(const std::string& timerId) -> void {
    if (m_timers.find(timerId) != m_timers.end()) {
        spdlog::warn("Timer with ID '{}' already exists, not overwriting.", timerId);
        return;
    }

    m_timers.emplace(timerId, std::chrono::high_resolution_clock::now());
}

auto Timers::getElapsed(const std::string& timerId) -> int64_t {
    auto it = m_timers.find(timerId);
    if (it == m_timers.end()) {
        return 0;
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);

    return time.count();
}

auto Timers::stop(const std::string& timerId) -> int64_t {
    auto it = m_timers.find(timerId);
    if (it == m_timers.end()) {
        return 0;
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
    m_timers.erase(it);

    return time.count();
}

void Timers::update() {
    auto now = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;
}

auto Timers::deltaTime() const -> float {
    return m_deltaTime;
}

}  // namespace Vengine