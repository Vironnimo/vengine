#pragma once

#include <chrono>
#include <map>
#include <string>

namespace Vengine {

class Timers {
   public:
    Timers();
    ~Timers();

    // TODO timer with end time and callback

    auto start(const std::string& timerId) -> void;
    auto getElapsed(const std::string& timerId) -> int64_t;
    auto stop(const std::string& timerId) -> int64_t;

    auto update() -> void;
    [[nodiscard]] auto deltaTime() const -> float;

   private:
    float m_deltaTime = 0.0f;

    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> m_timers;
};

}  // namespace Vengine