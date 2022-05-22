/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/CompositeCompare.h"
#include "Awl/KeyCompare.h"

#include "Awl/Testing/UnitTest.h"

#include <string>

namespace
{
    struct X
    {
        int a;
        std::string b;
    };

    inline auto MakeKey(const X& x)
    {
        return std::make_tuple(x.a, x.b);
    }

    using ACompare = awl::FieldCompare<X, int, &X::a>;
    using BCompare = awl::FieldCompare<X, std::string, &X::b>;

    X x1a{ 1, "a" };
    X x1b{ 1, "b" };
    X x2a{ 2, "a" };
    X x3a{ 3, "a" };
}

using namespace awl::testing;

AWT_TEST(CompositeCompare)
{
    AWT_UNUSED_CONTEXT;

    //auto comp = awl::CompositeCompare<X, ACompare, BCompare>(ACompare(), BCompare());
    auto comp = awl::compose_comparers<X>(ACompare(), BCompare());

    AWT_ASSERT(!comp(x1a, x1a));
    AWT_ASSERT(!comp(x1b, x1b));
    AWT_ASSERT(!comp(x2a, x2a));
    AWT_ASSERT(!comp(x3a, x3a));

    AWT_ASSERT(comp(x1a, x1b));
    AWT_ASSERT(!comp(x1b, x1a));

    AWT_ASSERT(comp(x2a, x3a));
    AWT_ASSERT(!comp(x3a, x2a));
}

AWT_TEST(TransparentCompositeCompare)
{
    AWT_UNUSED_CONTEXT;

    auto comp = awl::compose_transparent_comparers<X>(ACompare(), BCompare());

    AWT_ASSERT(!comp(x1a, x1a));
    AWT_ASSERT(!comp(x1b, x1b));
    AWT_ASSERT(!comp(x2a, x2a));
    AWT_ASSERT(!comp(x3a, x3a));

    AWT_ASSERT(comp(x1a, x1b));
    AWT_ASSERT(!comp(x1b, x1a));

    AWT_ASSERT(comp(x2a, x3a));
    AWT_ASSERT(!comp(x3a, x2a));

    AWT_ASSERT(!comp(x1a, MakeKey(x1a)));
    AWT_ASSERT(!comp(x1b, MakeKey(x1b)));
    AWT_ASSERT(!comp(x2a, MakeKey(x2a)));
    AWT_ASSERT(!comp(x3a, MakeKey(x3a)));

    AWT_ASSERT(comp(x1a, MakeKey(x1b)));
    AWT_ASSERT(!comp(x1b, MakeKey(x1a)));

    AWT_ASSERT(comp(x2a, MakeKey(x3a)));
    AWT_ASSERT(!comp(x3a, MakeKey(x2a)));

    AWT_ASSERT(!comp(MakeKey(x1a), x1a));
    AWT_ASSERT(!comp(MakeKey(x1b), x1b));
    AWT_ASSERT(!comp(MakeKey(x2a), x2a));
    AWT_ASSERT(!comp(MakeKey(x3a), x3a));

    AWT_ASSERT(comp(MakeKey(x1a), x1b));
    AWT_ASSERT(!comp(MakeKey(x1b), x1a));

    AWT_ASSERT(comp(MakeKey(x2a), x3a));
    AWT_ASSERT(!comp(MakeKey(x3a), x2a));
}
