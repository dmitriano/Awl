
#include <iostream>
#include <iomanip>
#include <map>

#include "Awl/IntRange.h"

#include "Awl/String.h"

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWL_TEST(IntRangeInt)
{
    AWL_UNUSED_CONTEXT;

    int n = 0;
    
    for (auto i : awl::make_count(10))
    {
        Assert::AreEqual(n++, i);
    }

    Assert::AreEqual(n, 10);
}

AWL_TEST(IntRangeSizeT)
{
    AWL_UNUSED_CONTEXT;

    size_t n = 0;

    const size_t N = 10;

    for (auto i : awl::make_count(N))
    {
        Assert::AreEqual(n++, i);
    }

    Assert::AreEqual(n, N);
}