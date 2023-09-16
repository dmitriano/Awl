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

using Map = std::map<int, std::string>;

using IntervalMap = awl::interval_map<int, std::string>;

static_assert(std::ranges::range<Map>);
static_assert(std::ranges::range<IntervalMap>);

AWT_TEST(IntervalMapIterator)
{
    AWT_UNUSED_CONTEXT;

    IntervalMap actual_map;

    {
        auto begin = actual_map.begin();
        auto end = actual_map.end();

        AWT_ASSERT(begin == end);
    }

    actual_map.assign(1, 3, "a");

    actual_map.assign(5, 6, "b");

    {
        auto begin = actual_map.begin();
        auto end = actual_map.end();

        AWT_ASSERT(begin != end);

        IntervalMap::const_iterator i = begin;

        //auto val = (*i).first;
        //auto val = (*i++).first;
        
        AWT_ASSERT((*i).second == "a");
        AWT_ASSERT(i != end && (*i++).first == 1);
        AWT_ASSERT((*i).second == "a");
        AWT_ASSERT(i != end && (*i++).first == 2);
        AWT_ASSERT((*i).second == "a");
        AWT_ASSERT(i != end && (*i++).first == 3);

        AWT_ASSERT((*i).second == "b");
        AWT_ASSERT(i != end && (*i++).first == 5);
        AWT_ASSERT((*i).second == "b");
        AWT_ASSERT(i != end && (*i++).first == 6);

        AWT_ASSERT(i == end);
    }
}

AWT_TEST(IntervalMap)
{
    AWT_UNUSED_CONTEXT;
    
    //AWT_ATTRIBUTE(int, range, 10);

    Map expected_map;

    IntervalMap actual_map;

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

    auto assert_equal = [&actual_map, &expected_map]()
    {
        // They are of a different types.
        auto pred = [](const IntervalMap::value_type& actual_pair, const Map::value_type& expected_pair) -> bool
        {
            return actual_pair.first == expected_pair.first && actual_pair.second == expected_pair.second;
        };

        AWT_ASSERT(std::ranges::equal(actual_map, expected_map, pred));
    };

    assign(1, 5, "a");

    assert_equal();

    assign(5, 6, "b");

    assert_equal();

    assign(3, 3, "c");

    assert_equal();

    assign(0, 10, "d");

    assert_equal();
}
