#pragma once

#include <chrono>

class Timer {
   public:
    static float getDeltaTime();
    static void updateDeltaTime();

   private:
    static std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    static float m_deltaTime;
};