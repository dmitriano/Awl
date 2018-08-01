#include <iostream>
#include <algorithm>

#include "Awl/ScopeGuard.h"

#include "UnitTesting.h"

using namespace UnitTesting;

AWL_TEST(ScopeGuard, Main)
{
    int usage = 1;

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; });

        guard.release();

        Assert::AreEqual(usage, 1);
    }

    Assert::AreEqual(usage, 1);

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; });

        Assert::AreEqual(usage, 1);
    }

    Assert::AreEqual(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; });

        Assert::AreEqual(usage, 1);
    }

    Assert::AreEqual(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; });

        guard.release();

        Assert::AreEqual(usage, 1);
    }

    Assert::AreEqual(usage, 1);

    usage = 0;

    //engaged = false

    {
        auto guard = awl::make_scope_guard([&usage]() { --usage; }, false);

        Assert::AreEqual(usage, 0);
    }

    Assert::AreEqual(usage, 0);

    {
        auto guard = awl::make_scope_guard(
            [&usage]() { ++usage; },
            [&usage]() { --usage; },
            false);

        Assert::AreEqual(usage, 0);
    }

    Assert::AreEqual(usage, 0);
}
