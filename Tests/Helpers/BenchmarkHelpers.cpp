/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <variant>
#include <array>

#include "Awl/TupleHelpers.h"
#include "Awl/Time.h"
#include "Awl/StringFormat.h"

#include "BenchmarkHelpers.h"
#include "FormattingHelpers.h"

namespace awl::testing::helpers
{
    template <typename value_type>
    value_type GetElapsedSeconds(std::chrono::steady_clock::duration d)
    {
        return std::chrono::duration_cast<std::chrono::duration<value_type>>(d).count();
    }

    template <class Func>
    double ReportValue(const TestContext & context, std::chrono::steady_clock::duration d, Func && func)
    {
        if (d == std::chrono::steady_clock::duration::zero())
        {
            context.logger.debug(_T("ZERO TIME"));
            return std::numeric_limits<double>::infinity();
        }

        awl::format message;
        message << _T("total time: ") << awl::duration_to_string(d) << _T(", ");

        const auto time = GetElapsedSeconds<double>(d);

        const double value = std::forward<Func>(func)(message, time);

        context.logger.debug(message);

        return value;
    }

    double ReportSpeed(const TestContext & context, std::chrono::steady_clock::duration d, size_t size)
    {
        return ReportValue(context, d, [size](awl::format & message, double time)
        {
            const double speed = size / time / (1024 * 1024);

            message << std::fixed << std::setprecision(2) << speed << _T(" MB/sec");

            return speed;
        });
    }

    double ReportCount(const TestContext & context, std::chrono::steady_clock::duration d, size_t count)
    {
        return ReportValue(context, d, [count](awl::format & message, double time)
        {
            const double speed = count / time;

            message << std::fixed << std::setprecision(2) << speed << _T(" elements/sec, (1 element takes ");

            if (count != 0)
            {
                std::chrono::nanoseconds ns(static_cast<std::chrono::nanoseconds::rep>(time / count * std::nano::den));

                message << awl::duration_to_string(ns);
            }
            else
            {
                message << _T("N/A");
            }

            message << _T("), total count : ") << count;

            return speed;
        });
    }

    void ReportCountAndSpeed(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t count, size_t size)
    {
        if (d == std::chrono::steady_clock::duration::zero())
        {
            context.logger.debug(_T("ZERO TIME"));
            return;
        }

        awl::format message;
        message << awl::duration_to_string(d) << _T(", ");

        const auto time = GetElapsedSeconds<double>(d);
        const double count_speed = count / time;

        message << std::fixed << std::setprecision(2) << count_speed << _T(" elements/sec, (1 element takes ");

        if (count != 0)
        {
            std::chrono::nanoseconds ns(static_cast<std::chrono::nanoseconds::rep>(time / count * std::nano::den));

            message << awl::duration_to_string(ns);
        }
        else
        {
            message << _T("N/A");
        }

        message << _T("), total count : ") << count << _T(", ");

        const double size_speed = size / time / (1024 * 1024);

        message << std::fixed << std::setprecision(2) << size_speed << _T(" MB/sec");

        context.logger.debug(message);
    }
}
