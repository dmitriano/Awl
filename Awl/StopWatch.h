#pragma once

#include <chrono>

namespace awl
{
    class StopWatch
    {
    public:

        StopWatch()
        {
            Reset();
        }

        void Reset()
        {
            startTime = std::chrono::steady_clock::now();
        }

        template <typename value_type, class ratio>
        value_type GetElapsedTime() const
        {
            return std::chrono::duration_cast<std::chrono::duration<value_type, ratio>>(GetElapsedNanoseconds()).count();
        }

        //The time is stored in integer nanoseconds but only the difference is of type float,
        //so the following calculation is accurate enought: diff = (2025 year + 1ns) - (2025 year).
        template <typename value_type>
        value_type GetElapsedSeconds() const
        {
            return std::chrono::duration_cast<std::chrono::duration<value_type>>(GetElapsedNanoseconds()).count();
        }

        std::chrono::steady_clock::duration GetElapsedNanoseconds() const
        {
            return std::chrono::steady_clock::now() - startTime;
        }

    private:

        std::chrono::steady_clock::time_point startTime;
    };
}
