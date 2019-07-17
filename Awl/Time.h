#pragma once

#include <ctime>
#include <chrono>

namespace awl
{
    template <class Clock, class Duration = typename Clock::duration>
    inline std::chrono::time_point<Clock, Duration> make_time(int year, int month, int day, int hour, int min, int second)
    {
        std::tm tm{};
        tm.tm_year = year - 1900;
        tm.tm_mon = month;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = min;
        tm.tm_sec = second;
        tm.tm_isdst = -1; //unknown

        const time_t t = std::mktime(&tm);

        return std::chrono::time_point_cast<Duration>(Clock::from_time_t(t));
    }

    template <class Clock, class Duration = typename Clock::duration>
    inline std::chrono::time_point<Clock, Duration> make_time(int year, int month, int day, int hour, int min, int second, Duration fs)
    {
        const auto tp = make_time<Clock, Duration>(year, month, day, hour, min, second);

        return tp + fs;
    }
}
