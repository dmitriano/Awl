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
        value_type GetElapsedCount() const
        {
            return std::chrono::duration_cast<std::chrono::duration<value_type, ratio>>(GetElapsedTime()).count();
        }

        template <class ToDuration>
        ToDuration GetElapsedCast() const
        {
            return std::chrono::duration_cast<ToDuration>(GetElapsedTime());
        }

        //The time is stored in integer nanoseconds but only the difference is of type float,
        //so the following calculation is accurate enought: diff = (2025 year + 1ns) - (2025 year).
        template <typename value_type>
        value_type GetElapsedSeconds() const
        {
            return std::chrono::duration_cast<std::chrono::duration<value_type>>(GetElapsedTime()).count();
        }

        std::chrono::steady_clock::duration GetElapsedTime() const
        {
            return std::chrono::steady_clock::now() - startTime;
        }

        operator std::chrono::steady_clock::duration() const
        {
            return GetElapsedTime();
        }

    private:

        std::chrono::steady_clock::time_point startTime;
    };
}
