#pragma once
#include <chrono>

namespace Astranox
{
    class Timer
    {
    public:
        Timer()
        {
            reset();
        }

        ~Timer() = default;

        void reset()
        {
            m_StartTime = std::chrono::high_resolution_clock::now();
        }

        float getElapsedSeconds()
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - m_StartTime).count() * 1e-9f;
        }

        float getElapsedMilliseconds()
        {
            return getElapsedSeconds() * 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
    };
}
