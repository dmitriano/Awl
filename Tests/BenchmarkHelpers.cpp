#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

#include "BenchmarkHelpers.h"

using namespace awl::testing;

namespace
{
    inline bool IsZero(std::chrono::steady_clock::duration d)
    {
        return d == std::chrono::steady_clock::duration::zero();
    }
    
    template <typename value_type>
    value_type GetElapsedSeconds(std::chrono::steady_clock::duration d)
    {
        return std::chrono::duration_cast<std::chrono::duration<value_type>>(d).count();
    }
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
    
    context.out << std::fixed << std::setprecision(2) << speed << _T(" MB/sec");

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

    context.out << std::fixed << std::setprecision(2) << speed << _T(" elements/sec");

    return speed;
}
