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
#include <vector>

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

    template <class T>
    struct SetChangeRecorder : awl::Observer<awl::INotifySetChanged<T>>
    {
        std::vector<T> added;
        std::vector<T> removed;
        size_t clearing_count = 0;

        void onAdded(const T& val) override
        {
            added.push_back(val);
        }

        void onRemoving(const T& val) override
        {
            removed.push_back(val);
        }

        void onClearing() override
        {
            ++clearing_count;
        }
    };

    template <class Set>
    void checkSetNotifications()
    {
        Set s;
        SetChangeRecorder<typename Set::value_type> recorder;

        s.subscribe(&recorder);

        AWL_ASSERT(s.insert(10).second);
        AWL_ASSERT(s.insert(20).second);
        AWL_ASSERT_FALSE(s.insert(10).second);

        AWL_ASSERT_EQUAL(size_t(2), recorder.added.size());
        AWL_ASSERT_EQUAL(10, recorder.added[0]);
        AWL_ASSERT_EQUAL(20, recorder.added[1]);
        AWL_ASSERT(recorder.removed.empty());
        AWL_ASSERT_EQUAL(size_t(0), recorder.clearing_count);

        auto i = s.find(10);
        AWL_ASSERT(i != s.end());
        s.erase(i);

        AWL_ASSERT_EQUAL(size_t(1), recorder.removed.size());
        AWL_ASSERT_EQUAL(10, recorder.removed[0]);
        AWL_ASSERT_EQUAL(size_t(0), recorder.clearing_count);

        s.clear();

        AWL_ASSERT_EQUAL(size_t(1), recorder.clearing_count);
        AWL_ASSERT(s.empty());
    }
}

AWL_TEST(ObservableSetAssignment)
{
    AWL_UNUSED_CONTEXT;

    using Compare = awl::member_compare<&A::key>;
    using Set = awl::observable_vector_set<A, Compare>;

    //Check if it satisfies the concept std::ranges::range.
    static_assert(std::ranges::range<Set>);

    Set s;
    s = {};
    s.insert(A(1));
    s = {};
    s.insert(A(1));
}

AWL_TEST(ObservableSetVector)
{
    AWL_UNUSED_CONTEXT;

    checkSetNotifications<awl::observable_vector_set<int>>();
}

AWL_TEST(ObservableSetUnordered)
{
    AWL_UNUSED_CONTEXT;

    checkSetNotifications<awl::observable_unordered_set<int>>();
}
