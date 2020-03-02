#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <variant>
#include <array>

#include "Awl/TupleHelpers.h"

#include "Helpers/BenchmarkHelpers.h"

namespace awl::testing::helpers
{
    using namespace awl::testing;

    inline bool IsZero(std::chrono::steady_clock::duration d)
    {
        return d == std::chrono::steady_clock::duration::zero();
    }
    
    template <typename value_type>
    inline value_type GetElapsedSeconds(std::chrono::steady_clock::duration d)
    {
        return std::chrono::duration_cast<std::chrono::duration<value_type>>(d).count();
    }

    using namespace std::chrono;

    using Durations = std::variant<hours, minutes, seconds, milliseconds, microseconds, nanoseconds>;
    std::array<const awl::Char *, std::variant_size_v<Durations>> durationUnits = {_T("hours"), _T("minutes"), _T("sec"), _T("ms"), _T("microseconds"), _T("ns")};

    template <size_t index>
    std::enable_if_t<(index < std::variant_size_v<Durations> - 1), void> PrintDuration(std::basic_ostream<awl::Char> & out, std::chrono::steady_clock::duration d)
    {
        using Duration = std::variant_alternative_t<index, Durations>;
        
        Duration cur = duration_cast<Duration>(d);

        if (cur == Duration::zero())
        {
            PrintDuration<index + 1>(out, d);
        }
        else
        {
            if (cur.count() < 100)
            {
                using NextDuration = std::variant_alternative_t<index + 1, Durations>;

                NextDuration next = duration_cast<NextDuration>(d);

                out << std::fixed << std::setprecision(2) << next.count() / 1000.0;
            }
            else
            {
                out << cur.count();
            }

            out << _T(" ") << durationUnits[index];
        }
    }

    template <size_t index>
    std::enable_if_t<(index == std::variant_size_v<Durations> - 1), void> PrintDuration(std::basic_ostream<awl::Char> & out, std::chrono::steady_clock::duration d)
    {
        if (d == std::chrono::steady_clock::duration::zero())
        {
            out << _T("ZERO TIME");
        }
        else
        {
            using Duration = std::variant_alternative_t<index, Durations>;

            Duration cur = duration_cast<Duration>(d);

            out << cur.count();

            out << _T(" ") << durationUnits[index];
        }
    }

    std::basic_ostream<awl::Char> & operator << (std::basic_ostream<awl::Char> & out, std::chrono::steady_clock::duration d)
    {
        PrintDuration<0>(out, d);

        return out;
    }

    double ReportSpeed(const TestContext & context, std::chrono::steady_clock::duration d, size_t size)
    {
        if (IsZero(d))
        {
            context.out << _T("ZERO TIME");
            return std::numeric_limits<double>::infinity();
        }

        const auto time = GetElapsedSeconds<double>(d);

        const double speed = size / time / (1024 * 1024);

        context.out << std::fixed << std::setprecision(2) << d << _T(", ") << speed << _T(" MB/sec");

        return speed;
    }

    double ReportCount(const TestContext & context, std::chrono::steady_clock::duration d, size_t count)
    {
        if (IsZero(d))
        {
            context.out << _T("ZERO TIME");
            return std::numeric_limits<double>::infinity();
        }

        const auto time = GetElapsedSeconds<double>(d);

        const double speed = count / time;

        context.out << std::fixed << std::setprecision(2) << d << _T(", ") << speed << _T(" elements/sec");

        return speed;
    }
}
