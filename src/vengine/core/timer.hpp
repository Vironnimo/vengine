#pragma once

#include <chrono>
#include <map>
#include <string>

namespace Vengine {

class Timer {
   public:
    Timer();
    ~Timer();

    // get current time in milliseconds
    auto time() -> int64_t;
    // start a new timer
    auto start(const std::string& timerId) -> void;
    // get elapsed time since start of the timer in milliseconds, without ending it
    auto get(const std::string& timerId) -> int64_t;
    // end timer and get time in milliseconds
    auto stop(const std::string& timerId) -> int64_t;

    auto update() -> void;
    auto deltaTime() -> float;

   private:
    float m_deltaTime = 0.0f;
    float m_fps = 0.0f;
    float m_fpsTimer = 0.0f;

    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> m_timers;
};

}  // namespace Vengine