/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Experimental/IntervalMap.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"

#include "Helpers/NonCopyable.h"

#include <deque>
#include <queue>
#include <ranges>

using Map = std::map<int, std::string>;

using IntervalMap = awl::interval_map<int, std::string>;

static_assert(std::ranges::range<Map>);
static_assert(std::ranges::range<IntervalMap>);

AWL_TEST(IntervalMapIterator)
{
    AWL_UNUSED_CONTEXT;

    IntervalMap actual_map;

    {
        auto begin = actual_map.begin();
        auto end = actual_map.end();

        AWL_ASSERT(begin == end);
    }

    actual_map.assign(1, 3, "a");

    actual_map.assign(5, 6, "b");

    {
        auto begin = actual_map.begin();
        auto end = actual_map.end();

        AWL_ASSERT(begin != end);

        IntervalMap::const_iterator i = begin;

        //auto val = (*i).first;
        //auto val = (*i++).first;
        
        AWL_ASSERT((*i).second == "a");
        AWL_ASSERT(i != end && (*i++).first == 1);
        AWL_ASSERT((*i).second == "a");
        AWL_ASSERT(i != end && (*i++).first == 2);
        AWL_ASSERT((*i).second == "a");
        AWL_ASSERT(i != end && (*i++).first == 3);

        AWL_ASSERT((*i).second == "b");
        AWL_ASSERT(i != end && (*i++).first == 5);
        AWL_ASSERT((*i).second == "b");
        AWL_ASSERT(i != end && (*i++).first == 6);

        AWL_ASSERT(i == end);
    }
}

AWL_TEST(IntervalMap)
{
    AWL_UNUSED_CONTEXT;
    
    AWL_ATTRIBUTE(int, range, 1000);
    AWL_ATTRIBUTE(size_t, iteration_count, 100);
    AWL_ATTRIBUTE(size_t, clear_count, 50);

    Map expected_map;

    IntervalMap actual_map;

    auto assert_equal = [&actual_map, &expected_map]()
    {
        // They are of a different types.
        auto pred = [](const IntervalMap::value_type& actual_pair, const Map::value_type& expected_pair) -> bool
        {
            return actual_pair.first == expected_pair.first && actual_pair.second == expected_pair.second;
        };

        AWL_ASSERT(std::ranges::equal(actual_map, expected_map, pred));
    };

    auto assign = [&context, &expected_map, &actual_map, &assert_equal](int a, int b, std::string value)
    {
        //context.out << _T("Assigning: [") << a << _T(", ") << b << "] = " << awl::FromAString(value) << std::endl;
        
        // value is moved here
        actual_map.assign(a, b, value);

        for (int i = a; i <= b; ++i)
        {
            AWL_ASSERT(actual_map.at(i) == value);

            expected_map[i] = value;
        }

        //size_t index = 0;
        
        //for (auto [key, val] : actual_map)
        //{
        //    context.out << _T("#") << index++ << _T("\t") << key << _T(" => ") << awl::FromAString(val) << std::endl;
        //}

        assert_equal();
    };

    auto clear = [&expected_map, &actual_map, &assert_equal]()
    {
        actual_map.clear();
        expected_map.clear();

        assert_equal();
    };

    assert_equal();

    //assign(1, 5, "a");

    //assign(5, 6, "b");

    //assign(3, 3, "c");

    //assign(0, 10, "d");

    //assign(5, 6, "e");

    //assign(12, 13, "f");

    //assign(3, 5, "g");

    std::uniform_int_distribution<int> a_dist(0, range);

    for (size_t i = 0; i < iteration_count; ++i)
    {
        if (iteration_count % clear_count == 0)
        {
            clear();
        }

        const int a = a_dist(awl::random());

        std::uniform_int_distribution<int> len_dist(0, range - a);

        const int len = len_dist(awl::random());

        const char ch = 'A' + (i % ('Z' - 'A'));

        std::string val(1, ch);

        assign(a, a + len, val);
    }
}
