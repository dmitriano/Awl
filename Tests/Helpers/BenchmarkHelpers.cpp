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
            context.out << _T("ZERO TIME");
            return std::numeric_limits<double>::infinity();
        }

        context.out << _T("total time: ");
        
        format_duration(context.out, d);

        context.out << _T(", ");

        const auto time = GetElapsedSeconds<double>(d);

        return func(time);
    }
    
    double ReportSpeed(const TestContext & context, std::chrono::steady_clock::duration d, size_t size)
    {
        return ReportValue(context, d, [&context, size](double time)
        {
            const double speed = size / time / (1024 * 1024);

            context.out << std::fixed << std::setprecision(2) << speed << _T(" MB/sec");

            return speed;
        });
    }

    double ReportCount(const TestContext & context, std::chrono::steady_clock::duration d, size_t count)
    {
        return ReportValue(context, d, [&context, count](double time)
        {
            const double speed = count / time;

            context.out << std::fixed << std::setprecision(2) << speed << _T(" elements/sec, (1 element takes ");

            if (count != 0)
            {
                std::chrono::nanoseconds ns(static_cast<std::chrono::nanoseconds::rep>(time / count * std::nano::den));

                format_duration(context.out, ns);
            }
            else
            {
                context.out << _T("N/A");
            }
            
            context.out << _T("), total count : ") << count;

            return speed;
        });
    }

    void ReportCountAndSpeed(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t count, size_t size)
    {
        context.out << d << _T(", ");
        ReportCount(context, d, count);
        context.out << _T(", ");
        ReportSpeed(context, d, size);
    }
}
