/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Experimental/Destructible.h"
#include "Tests/Helpers/NonCopyable.h"

#include "Awl/ScopeGuard.h"
#include "Awl/Testing/UnitTest.h"
namespace
{
    constexpr int value = 5;

    using A = awl::testing::helpers::NonCopyable;
}

AWL_TEST(Destructible)
{
    AWL_UNUSED_CONTEXT;
    
    AWL_ASSERT_EQUAL(0, A::count);

    {
        awl::Destructible<A> da(value);

        auto guard = awl::make_scope_guard([&da]() { da.destroy();  });

        AWL_ASSERT(*da == A(value));
    }

    AWL_ASSERT_EQUAL(0, A::count);
}
