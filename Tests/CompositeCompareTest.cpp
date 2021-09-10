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

    using ACompare = awl::FieldCompare<X, int, &X::a>;
    using BCompare = awl::FieldCompare<X, std::string, &X::b>;
}

using namespace awl::testing;

AWT_TEST(CompositeCompare)
{
    AWT_UNUSED_CONTEXT;

    X x1a{ 1, "a" };
    X x1b{ 1, "b" };
    X x2a{ 2, "a" };
    X x3a{ 3, "a" };

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
