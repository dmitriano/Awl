/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Aggregator.h"

#include "Awl/Testing/UnitTest.h"

#include "Helpers/NonCopyable.h"

#include <spdlog/fmt/fmt.h>

using A = awl::testing::helpers::NonCopyable;

AWL_TEST(Aggregator)
{
    AWL_UNUSED_CONTEXT;

    bool called = false;
    
    awl::aggregator a(std::function([&called](int i, char c, A a)
    {
        AWL_ASSERT_EQUAL(5, i);
        AWL_ASSERT_EQUAL('a', c);
        AWL_ASSERT(a == A(25));

        called = true;
    }));

    AWL_ASSERT(!a.all());
    AWL_ASSERT(!called);

    a.set<0>(5);

    AWL_ASSERT(!a.all());
    AWL_ASSERT(!called);

    a.set<1>('a');

    AWL_ASSERT(!a.all());
    AWL_ASSERT(!called);

    a.set<2>(A(25));

    AWL_ASSERT(a.all());
    AWL_ASSERT(called);
}
