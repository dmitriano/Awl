/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/StaticChain.h"
#include "Awl/StringFormat.h"

#include "Awl/Testing/UnitTest.h"

#include "Helpers/NonCopyable.h"

namespace
{
    //using X = awl::testing::helpers::NonCopyable;
    using X = int;

    awl::StaticLink<X> x1(_T("0"), 0);
    awl::StaticLink<X> x2(_T("1"), 1);
    awl::StaticLink<X> x3(_T("2"), 2);
}

AWT_TEST(StaticChain)
{
    AWT_UNUSED_CONTEXT;

    for (X val = 0; val < 3; ++val)
    {
        auto i = awl::static_chain<X>().find(awl::format() << val);
        
        AWT_ASSERT(i != awl::static_chain<X>().end());
        AWT_ASSERT(i->value() == val);
    }
}
