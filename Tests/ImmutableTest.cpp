/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Experimental/Immutable.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"
#include "Awl/IntRange.h"

#include <vector>

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

AWL_TEST(ImmutableVector)
{
    AWL_ATTRIBUTE(size_t, initial_size, 0);
    AWL_ATTRIBUTE(size_t, insert_count, 1000);

    std::vector<awl::immutable<A>> v;

    const std::string long_string = "A very long string that is not copied, but moved.";

    v.resize(initial_size, awl::make_immutable<A>(1, long_string));

    for (size_t i = 0; i < insert_count; ++i)
    {
        std::ostringstream out;

        out << long_string << " " << i;

        v.push_back(awl::make_immutable<A>(2, out.str()));
    }
}

#ifdef AWL_DEBUG_IMMUTABLE

namespace
{
    void func(const awl::testing::TestContext& context, awl::immutable<A> a)
    {
        context.logger.debug(awl::format() << a->x << " " << a->y);
    }
}

AWL_TEST(ImmutableException)
{
    awl::immutable<A> a = awl::make_immutable<A>(5, "abc");

    func(context, std::move(a));

    bool thrown = false;

    try
    {
        int x = a->x;

        context.logger.debug(awl::format() << a->x);
    }
    catch (const std::exception& e)
    {
        thrown = true;

        context.logger.debug(e.what());
    }

    AWL_ASSERT(thrown);
}

#endif