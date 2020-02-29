#pragma once

#include "Awl/Testing/UnitTest.h"

#include <chrono>

namespace awl::testing::helpers
{
    double ReportSpeed(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t size);

    double ReportCount(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t count);

    inline void ReportCountAndSpeed(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t count, size_t size)
    {
        ReportCount(context, d, count);
        context.out << _T(", ");
        ReportSpeed(context, d, size);
    }
}
