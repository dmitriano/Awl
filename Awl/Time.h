/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Separator.h"
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

    constexpr uint8_t default_duration_part_count = 2;

    template<class C, class Rep, class Period>
    void format_duration(std::basic_ostream<C>& out, std::chrono::duration<Rep, Period> val, uint8_t part_count = default_duration_part_count)
    {
        using namespace std::chrono;

#if defined(_MSC_VER) || (defined(__GNUC__) && !defined(__clang__))

        static_cast<void>(part_count);

        //GCC13 have this.
        std::chrono::hh_mm_ss formatted{ val };

        out << formatted;

// TODO: Check if CLang support std::chrono::hh_mm_ss.
#elif !defined(__clang__)

        awl::basic_separator<C> sep(':');

        uint8_t count = 0;

        std::chrono::duration<Rep, Period> left = val;

        auto print = [&out, &sep, &left, &count, part_count]<typename Duration>()
        {
            if (count < part_count)
            {
                const Duration s = duration_cast<Duration>(left);

                if (s != Duration::zero())
                {
                    //This requires GCC12.
                    out << sep << s;

                    left -= duration_cast<std::chrono::duration<Rep, Period>>(s);

                    if constexpr (Duration::period::den > 1)
                    {
                        ++count;
                    }
                }
            }
        };

        print.template operator()<years>();

        print.template operator()<months>();

        print.template operator()<days>();

        print.template operator()<hours>();

        print.template operator()<minutes>();

        print.template operator()<seconds>();

        print.template operator()<milliseconds>();

        print.template operator()<microseconds>();

        print.template operator()<nanoseconds>();

#else

        static_cast<void>(part_count);

        nanoseconds ns = val;

        out << ns.count() << _T("ns");

#endif
    }

    //It did not compile for some unknown reason.
    template<class C, class Rep, class Period>
    std::basic_string<C> duration_to_string(std::chrono::duration<Rep, Period> d, uint8_t part_count = default_duration_part_count)
    {
        std::basic_ostringstream<C> out;

        awl::format_duration(out, d, part_count);

        return out.str();
    }

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const StopWatch& sw)
    {
        format_duration(out, sw.GetElapsedTime(), default_duration_part_count);
        
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
