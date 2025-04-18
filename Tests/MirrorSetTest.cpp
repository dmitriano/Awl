/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/MirrorSet.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"
#include "Awl/String.h"
#include "Awl/KeyCompare.h"
#include "Awl/Tuplizable.h"

using namespace awl::testing;

namespace
{
    struct A
    {
        size_t key1;
        size_t key2;
        size_t value;

        AWL_TUPLIZABLE(key1, key2, value)
    };

    AWL_MEMBERWISE_EQUATABLE(A)

    using Compare1 = awl::member_compare<&A::key1>;
    using Compare2 = awl::member_compare<&A::key2>;
    
    using Set = awl::observable_set<A, Compare1>;
    using MirrorSet = awl::mirror_set<A, Compare2>;

    void AssertEqual(const Set& s, const MirrorSet& ms)
    {
        Set s1;

        for (auto& a : ms)
        {
            s1.insert(a);
        }

        AWL_ASSERT(s == s1);
    }

    void AssertEqual(const Set& s, const MirrorSet& ms1, const MirrorSet& ms2)
    {
        AssertEqual(s, ms1);
        AssertEqual(s, ms2);
    }
}

AWL_TEST(MirrorSet)
{
    AWL_UNUSED_CONTEXT;

    Set s;

    MirrorSet mirror1;

    MirrorSet mirror2;

    auto assert_equal = [&s, &mirror1, &mirror2]
    {
        AssertEqual(s, mirror1, mirror2);
    };
        
    assert_equal();

    mirror1.reflect(s);
    mirror2.reflect(mirror1);

    assert_equal();

    //This will unsubscribe.
    //s = {};

    s.clear();

    assert_equal();

    auto [i1, added] = s.insert({ 1, 2, 100 });

    assert_equal();

    s.insert({ 3, 4, 101 });

    assert_equal();

    s.erase(i1);

    assert_equal();

    s.clear();

    assert_equal();

    s.insert({ 3, 4, 101 });

    //Move assignment unsubscribes.
    s = {};

    s.insert({ 5, 6, 102 });

    //We stopped to receive the notifications.
    AWL_ASSERT_EQUAL(0u, mirror1.size());
    AWL_ASSERT_EQUAL(0u, mirror2.size());
}
