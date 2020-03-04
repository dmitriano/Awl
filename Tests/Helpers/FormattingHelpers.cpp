#include <iostream>
#include <iomanip>
#include <variant>
#include <array>

#include "Awl/TupleHelpers.h"

#include "FormattingHelpers.h"

namespace awl::testing::helpers
{
    using namespace std::chrono;

    using Durations = std::variant<hours, minutes, seconds, milliseconds, microseconds, nanoseconds>;

    template <size_t index>
    inline void PrintDuration(awl::ostream & out, std::chrono::steady_clock::duration d)
    {
        constexpr std::array<const awl::Char *, std::variant_size_v<Durations>> durationUnits = { _T("h"), _T("min"), _T("s"), _T("ms"), _T("us"), _T("ns") };

        using Duration = std::variant_alternative_t<index, Durations>;
        
        Duration cur = duration_cast<Duration>(d);

        if constexpr (index == std::variant_size_v<Durations> - 1)
        {
            if (d == std::chrono::steady_clock::duration::zero())
            {
                out << _T("ZERO TIME");
            }
            else
            {
                out << cur.count() << durationUnits[index];
            }
        }
        else
        {
            if (cur == Duration::zero())
            {
                PrintDuration<index + 1>(out, d);
            }
            else
            {
                out << cur.count() << durationUnits[index];

                if (cur.count() < 10)
                {
                    using NextDuration = std::variant_alternative_t<index + 1, Durations>;

                    NextDuration next = duration_cast<NextDuration>(d) - duration_cast<NextDuration>(cur);

                    out << _T(":") << next.count() << durationUnits[index + 1];
                }
            }
        }
    }

    awl::ostream & operator << (awl::ostream & out, std::chrono::steady_clock::duration d)
    {
        PrintDuration<0>(out, d);

        return out;
    }
}
