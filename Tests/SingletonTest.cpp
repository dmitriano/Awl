#include "Experimental/Destructible.h"
#include "Awl/ScopeGuard.h"
#include "Awl/Testing/UnitTest.h"

#include "Helpers/NonCopyable.h"

using A = awl::testing::helpers::NonCopyable;

AWT_TEST(Destructible)
{
    AWT_UNUSED_CONTEXT;
    
    AWT_ASSERT_EQUAL(0, A::count);

    {
        awl::Destructible<A> da(5);

        auto guard = awl::make_scope_guard([&da]() { da.Destroy();  });

        AWT_ASSERT(*da == A(5));
    }

    AWT_ASSERT_EQUAL(0, A::count);
}
