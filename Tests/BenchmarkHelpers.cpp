#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

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

    /*
    template <class Duration>
    const awl::Char * DurationUnit;
    
    template <class Duration>
    void FormatDuration(Duration d)
    {
        using namespace std::chrono;

        if (ms.count() < 100)
        {
            out << std::fixed << std::setprecision(2) << duration_cast<microseconds>(d).count() / 1000.0;
        }
        else
        {
            out << ms.count();
        }

        out << DurationUnit<Duration>;
    }
    */
        
    std::basic_ostream<awl::Char> & operator << (std::basic_ostream<awl::Char> & out, std::chrono::steady_clock::duration d)
    {
        using namespace std::chrono;
        
        seconds s = duration_cast<seconds>(d);

        if (s == seconds::zero())
        {
            milliseconds ms = duration_cast<milliseconds>(d);

            if (ms == milliseconds::zero())
            {
                microseconds mms = duration_cast<microseconds>(d);

                if (mms.count() < 100)
                {
                    out << std::fixed << std::setprecision(2) << duration_cast<nanoseconds>(d).count() / 1000.0;
                }
                else
                {
                    out << mms.count();
                }

                out << _T(" microseconds");
            }
            else
            {
                if (ms.count() < 100)
                {
                    out << std::fixed << std::setprecision(2) << duration_cast<microseconds>(d).count() / 1000.0;
                }
                else
                {
                    out << ms.count();
                }

                out << _T(" ms");
            }
        }
        else
        {
            out << std::fixed << std::setprecision(2) << GetElapsedSeconds<double>(d) << _T(" sec");
        }

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
