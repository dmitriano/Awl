/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
        AWL_ASSERT_EQUAL(n++, i);
    }

    AWL_ASSERT_EQUAL(n, 10);

    const std::vector<int> sample{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    const auto range = awl::make_count(10);
    
    const std::vector<int> v(range.begin(), range.end());

    AWL_ASSERT(v == sample);
}

AWL_TEST(IntRangeSizeT)
{
    AWL_UNUSED_CONTEXT;

    size_t n = 0;

    const size_t N = 10;

    for (auto i : awl::make_count(N))
    {
        AWL_ASSERT_EQUAL(n++, i);
    }

    AWL_ASSERT_EQUAL(n, N);
}
