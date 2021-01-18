#include "Awl/Aggregator.h"

#include "Awl/Testing/UnitTest.h"

AWT_TEST(Aggregator)
{
    AWT_UNUSED_CONTEXT;

    bool called = false;
    
    awl::aggregator a(std::function([&called](int i, char c)
    {
        AWT_ASSERT_EQUAL(5, i);
        AWT_ASSERT_EQUAL('a', c);

        called = true;
    }));

    AWT_ASSERT(!a.all());
    AWT_ASSERT(!called);

    a.set<0u>(5);

    AWT_ASSERT(!a.all());
    AWT_ASSERT(!called);

    a.set<1u>('a');

    AWT_ASSERT(a.all());
    AWT_ASSERT(called);
}
