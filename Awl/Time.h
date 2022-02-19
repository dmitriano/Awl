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

        awl::separator sep(_T(":"));

        uint8_t count = 0;

        std::chrono::duration<Rep, Period> left = val;

        auto print = [&out, &sep, &left, &count, part_count]<typename Duration>()
        {
            if (count < part_count)
            {
                const Duration s = duration_cast<Duration>(left);

                if (s != Duration::zero())
                {
                    out << sep << s;

                    left -= s;

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
    }

    //It did not compile for some unknown reason.
    //template<class C, class Rep, class Period>
    //std::basic_string<C> duration_to_string(std::chrono::duration<Rep, Period> d, uint8_t part_count = default_duration_part_count)
    //{
    //    std::basic_ostringstream<C> out;

    //    awl::format_duration(out, d, part_count);

    //    return out.str();
    //}

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const StopWatch& sw)
    {
        format_duration(out, sw.GetElapsedTime(), default_duration_part_count);
        
        return out;
    }
}
