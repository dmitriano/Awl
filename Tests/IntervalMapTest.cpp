/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Experimental/IntervalMap.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

#include "Helpers/NonCopyable.h"

#include <deque>
#include <queue>
#include <ranges>

AWT_TEST(IntervalMap)
{
    AWT_UNUSED_CONTEXT;
    
    //AWT_ATTRIBUTE(int, range, 10);

    awl::interval_map<int, std::string> im;

    im.assign(5, 9, std::string("a"));

    AWT_ASSERT(im.at(7) == "a");
}
