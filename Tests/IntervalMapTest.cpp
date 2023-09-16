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

    std::map<int, std::string> expected_map;

    awl::interval_map<int, std::string> actual_map;

    auto assign = [&expected_map, &actual_map](int a, int b, std::string value)
    {
        const std::string saved_value = value;
        
        // value is moved here
        actual_map.assign(a, b, value);

        for (int i = a; i != b; ++i)
        {
            AWT_ASSERT(actual_map.at(i) == saved_value);

            expected_map.emplace(i, saved_value);
        }
    };

    assign(1, 5, "a");
}
