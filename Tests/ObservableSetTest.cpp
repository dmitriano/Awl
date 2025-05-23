/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/ObservableSet.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Random.h"
#include "Awl/String.h"
#include "Awl/KeyCompare.h"
#include "Awl/Tuplizable.h"

#include <ranges>

using namespace awl::testing;

namespace
{
    struct A
    {
        A() = default;

        explicit A(size_t k) : key(k), attribute(k + 1)
        {
        }

        size_t key;
        size_t attribute;

        size_t GetKey() const
        {
            return key;
        }

        const size_t & GetKeyRef() const
        {
            return key;
        }

        //for testing
        AWL_TUPLIZABLE(key)
    };
}

AWL_TEST(ObservableSetAssignment)
{
    AWL_UNUSED_CONTEXT;

    using Compare = awl::member_compare<&A::key>;
    using Set = awl::observable_set<A, Compare>;

    //Check if it satisfies the concept std::ranges::range.
    static_assert(std::ranges::range<Set>);

    Set s;
    s = {};
    s.insert(A(1));
    s = {};
    s.insert(A(1));
}
