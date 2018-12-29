#include <iostream>
#include <iomanip>

#include "BenchmarkHelpers.h"

using namespace awl::testing;

double ReportSpeed(const TestContext & context, const awl::StopWatch & w, size_t size)
{
    const auto time = w.GetElapsedSeconds<double>();

    const double speed = size / time / (1024 * 1024);
    
    context.out << std::fixed << std::setprecision(2) << speed << _T(" MB/sec");

    return speed;
}

double ReportCount(const TestContext & context, const awl::StopWatch & w, size_t count)
{
    const auto time = w.GetElapsedSeconds<double>();

    const double speed = count / time;

    context.out << std::fixed << std::setprecision(2) << speed << _T(" elements/sec");

    return speed;
}
