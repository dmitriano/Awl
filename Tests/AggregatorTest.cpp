#include "Awl/Aggregator.h"

#include "Awl/Testing/UnitTest.h"

#include "Helpers/NonCopyable.h"

using A = awl::testing::helpers::NonCopyable;

AWT_TEST(Aggregator)
{
    AWT_UNUSED_CONTEXT;

    bool called = false;
    
    awl::aggregator a(std::function([&called](int i, char c, A a)
    {
        AWT_ASSERT_EQUAL(5, i);
        AWT_ASSERT_EQUAL('a', c);
        AWT_ASSERT(a == A(25));

        called = true;
    }));

    AWT_ASSERT(!a.all());
    AWT_ASSERT(!called);

    a.set<0>(5);

    AWT_ASSERT(!a.all());
    AWT_ASSERT(!called);

    a.set<1>('a');

    AWT_ASSERT(!a.all());
    AWT_ASSERT(!called);

    a.set<2>(A(25));

    AWT_ASSERT(a.all());
    AWT_ASSERT(called);
}
