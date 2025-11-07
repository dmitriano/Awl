/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Experimental/Immutable.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

namespace
{
    struct A
    {
        int x;
        std::string y;

        bool operator ==(const A& other) const = default;
    };
}

AWL_TEST(ImmutableConstructorAndOperators)
{
    awl::immutable<A> a1 = awl::make_immutable<A>(5, "abc");

    context.logger.debug(awl::format() << a1->x << " " << a1->y);

    awl::immutable<A> a2 = a1;

    context.logger.debug(awl::format() << a2->x << " " << a2->y);

    AWL_ASSERT(a2 == a1);
}

AWL_TEST(ImmutableConstructor)
{
}
