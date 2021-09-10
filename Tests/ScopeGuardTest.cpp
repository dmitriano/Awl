/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <algorithm>

#include "Awl/ScopeGuard.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWT_TEST(ScopeGuard)
{
    AWT_UNUSED_CONTEXT;

    int usage = 1;

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; });

        guard.release();

        AWT_ASSERT_EQUAL(usage, 1);
    }

    AWT_ASSERT_EQUAL(usage, 1);

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; });

        AWT_ASSERT_EQUAL(usage, 1);
    }

    AWT_ASSERT_EQUAL(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; });

        AWT_ASSERT_EQUAL(usage, 1);
    }

    AWT_ASSERT_EQUAL(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; });

        guard.release();

        AWT_ASSERT_EQUAL(usage, 1);
    }

    AWT_ASSERT_EQUAL(usage, 1);

    usage = 0;

    //engaged = false

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; }, false);

        AWT_ASSERT_EQUAL(usage, 0);
    }

    AWT_ASSERT_EQUAL(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; },
            false);

        AWT_ASSERT_EQUAL(usage, 0);
    }

    AWT_ASSERT_EQUAL(usage, 0);

    {
        auto init = [&usage]() { ++usage; };
        auto free = [&usage]() { --usage; };

        auto guard = awl::make_scope_guard(init, free, true);
    }

    AWT_ASSERT_EQUAL(usage, 0);
}
