/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <algorithm>

#include "Awl/ScopeGuard.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWL_TEST(ScopeGuard)
{
    AWL_UNUSED_CONTEXT;

    int usage = 1;

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; });

        guard.release();

        AWL_ASSERT_EQUAL(usage, 1);
    }

    AWL_ASSERT_EQUAL(usage, 1);

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; });

        AWL_ASSERT_EQUAL(usage, 1);
    }

    AWL_ASSERT_EQUAL(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; });

        AWL_ASSERT_EQUAL(usage, 1);
    }

    AWL_ASSERT_EQUAL(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; });

        guard.release();

        AWL_ASSERT_EQUAL(usage, 1);
    }

    AWL_ASSERT_EQUAL(usage, 1);

    usage = 0;

    //engaged = false

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; }, false);

        AWL_ASSERT_EQUAL(usage, 0);
    }

    AWL_ASSERT_EQUAL(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; },
            false);

        AWL_ASSERT_EQUAL(usage, 0);
    }

    AWL_ASSERT_EQUAL(usage, 0);

    {
        auto init = [&usage]() { ++usage; };
        auto free = [&usage]() { --usage; };

        auto guard = awl::make_scope_guard(init, free, true);
    }

    AWL_ASSERT_EQUAL(usage, 0);
}
