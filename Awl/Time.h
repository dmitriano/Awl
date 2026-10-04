/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Duration.h"
#include "Awl/StopWatch.h"

#include <ctime>
#include <chrono>

namespace awl
{
    template <class Clock, class Duration = typename Clock::duration>
    std::chrono::time_point<Clock, Duration> make_time(int year, int month, int day, int hour, int min, int second)
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

    template <class Clock, class Duration>
    std::chrono::time_point<Clock, Duration> make_time(int year, int month, int day, int hour, int min, int second, Duration fs)
    {
        const auto tp = make_time<Clock, Duration>(year, month, day, hour, min, second);

        return tp + fs;
    }

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const StopWatch& sw)
    {
        format_duration(out, sw.elapsedTime(), default_duration_part_count);
        
        return out;
    }
}

// Looks like both __GNUC__ and __clang__ are defined in Apple Clang.
#if (defined(__GNUC__) && defined(__clang__)) && !defined(__APPLE__) && __clang_major__ < 20
namespace std
{
    template <class C, class Clock, class Duration = typename Clock::duration>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const std::chrono::time_point<Clock, Duration>& tp)
    {
        const std::time_t t = Clock::to_time_t(tp);

        return out << std::ctime(&t);
    }
}
#endif

#if defined(__GNUC__) && defined(__clang__)
namespace std
{
    template<class C, class Rep, class Period>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const std::chrono::duration<Rep, Period>& d)
    {
        using namespace awl;

        format_duration(out, d, default_duration_part_count);

        return out;
    }
}
#endif
