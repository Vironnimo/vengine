#include "timer.hpp"

#include <chrono>
#include <spdlog/spdlog.h>

namespace Vengine {

Timer::Timer() : m_lastFrameTime(std::chrono::high_resolution_clock::now()) {
    spdlog::debug("Constructor Timer");
}

Timer::~Timer() {
    spdlog::debug("Destructor Timer");
}

auto Timer::time() -> int64_t {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now);

    return time.count();
}

auto Timer::start(const std::string& timerId) -> void {
    if (m_timers.find(timerId) != m_timers.end()) {
        spdlog::warn("Timer with ID '{}' already exists.", timerId);
        return;
    }

    m_timers.emplace(timerId, std::chrono::high_resolution_clock::now());
}

auto Timer::get(const std::string& timerId) -> int64_t {
    auto it = m_timers.find(timerId);
    if (it == m_timers.end()) {
        return 0;
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);

    return time.count();
}

auto Timer::stop(const std::string& timerId) -> int64_t {
    auto it = m_timers.find(timerId);
    if (it == m_timers.end()) {
        return 0;
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
    m_timers.erase(it);

    return time.count();
}

void Timer::update() {
    auto now = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;
}

auto Timer::deltaTime() -> float {
    return m_deltaTime;
}

}  // namespace Vengine