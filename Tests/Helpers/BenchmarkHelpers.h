/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/UnitTest.h"

#include <chrono>

namespace awl::testing::helpers
{
    double ReportSpeed(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t size);

    double ReportCount(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t count);

    void ReportCountAndSpeed(const awl::testing::TestContext & context, std::chrono::steady_clock::duration d, size_t count, size_t size);
}
