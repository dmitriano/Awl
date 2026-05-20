/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/ObservableSet.h"
#include "Awl/ObservableUnorderedSet.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/KeyEqual.h"
#include "Awl/KeyHash.h"
#include "Awl/Random.h"
#include "Awl/String.h"
#include "Awl/KeyCompare.h"
#include "Awl/Tuplizable.h"

#include <memory>
#include <ranges>
#include <unordered_set>
#include <vector>

using namespace awl::testing;

namespace
{
    struct A
    {
        A() = default;

        explicit A(size_t k) : key(k), attribute(k + 1)
        {}

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

    template <class Set, class ReferenceSet>
    void assertSameContent(const Set& actual, const ReferenceSet& expected)
    {
        AWL_ASSERT_EQUAL(expected.size(), actual.size());

        for (const auto& val : expected)
        {
            AWL_ASSERT(actual.contains(val));
        }

        for (const auto& val : actual)
        {
            AWL_ASSERT(expected.contains(val));
        }
    }

    template <class Set, class ReferenceSet>
    void checkSetNotifications()
    {
        Set s;
        ReferenceSet expected;
        SetChangeRecorder<typename Set::value_type> recorder;

        s.subscribe(&recorder);

        AWL_ASSERT(s.insert(10).second);
        expected.insert(10);
        assertSameContent<Set, ReferenceSet>(s, expected);

        AWL_ASSERT(s.insert(20).second);
        expected.insert(20);
        assertSameContent<Set, ReferenceSet>(s, expected);

        AWL_ASSERT_FALSE(s.insert(10).second);
        assertSameContent<Set, ReferenceSet>(s, expected);

        AWL_ASSERT_EQUAL(size_t(2), recorder.added.size());
        AWL_ASSERT_EQUAL(10, recorder.added[0]);
        AWL_ASSERT_EQUAL(20, recorder.added[1]);
        AWL_ASSERT(recorder.removed.empty());
        AWL_ASSERT_EQUAL(size_t(0), recorder.clearing_count);

        auto i = s.find(10);
        AWL_ASSERT(i != s.end());
        s.erase(i);
        expected.erase(10);
        assertSameContent<Set, ReferenceSet>(s, expected);

        AWL_ASSERT_EQUAL(size_t(1), recorder.removed.size());
        AWL_ASSERT_EQUAL(10, recorder.removed[0]);
        AWL_ASSERT_EQUAL(size_t(0), recorder.clearing_count);

        s.clear();
        expected.clear();
        assertSameContent<Set, ReferenceSet>(s, expected);

        AWL_ASSERT_EQUAL(size_t(1), recorder.clearing_count);
        AWL_ASSERT(s.empty());
    }
}

AWL_TEST(ObservableSetAssignment)
{
    AWL_UNUSED_CONTEXT;

    using Compare = awl::KeyCompare<A, &A::key>;
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

    checkSetNotifications<awl::observable_vector_set<int>, awl::vector_set<int>>();
}

AWL_TEST(ObservableSetUnordered)
{
    AWL_UNUSED_CONTEXT;

    checkSetNotifications<awl::observable_unordered_set<int>, std::unordered_set<int>>();
}

AWL_TEST(ObservableSetUnorderedSharedKey)
{
    AWL_UNUSED_CONTEXT;

    using Set = awl::observable_unordered_set<
        std::shared_ptr<A>,
        awl::KeyHash<std::shared_ptr<A>, &A::GetKey>,
        awl::KeyEqual<std::shared_ptr<A>, &A::GetKey>>;

    Set set;

    std::shared_ptr<A> first = std::make_shared<A>(10);
    std::shared_ptr<A> second = std::make_shared<A>(20);

    AWL_ASSERT(set.insert(first).second);
    AWL_ASSERT(set.insert(second).second);
    AWL_ASSERT_FALSE(set.insert(std::make_shared<A>(10)).second);

    AWL_ASSERT(set.contains(10u));
    AWL_ASSERT(set.contains(first));

    auto i = set.find(20u);

    AWL_ASSERT(i != set.end());
    AWL_ASSERT(*i == second);
}

AWL_TEST(ObservableSetUnorderedStdInterface)
{
    AWL_UNUSED_CONTEXT;

    using Set = awl::observable_unordered_set<int>;

    static_assert(std::ranges::range<Set>);

    Set set = { 1, 2 };

    AWL_ASSERT(set.key_eq()(1, 1));
    AWL_ASSERT_EQUAL(std::hash<int>{}(1), set.hash_function()(1));

    set.reserve(16);
    AWL_ASSERT(set.bucket(1) < set.bucket_count());

    SetChangeRecorder<int> recorder;
    set.subscribe(&recorder);

    Set::node_type node = set.extract(1);
    AWL_ASSERT(!node.empty());
    AWL_ASSERT_FALSE(set.contains(1));

    const Set::insert_return_type insert_result = set.insert(std::move(node));
    AWL_ASSERT(insert_result.inserted);
    AWL_ASSERT(set.contains(1));

    std::unordered_set<int> source = { 3, 4 };
    set.merge(source);

    AWL_ASSERT(source.empty());
    AWL_ASSERT(set.contains(3));
    AWL_ASSERT(set.contains(4));

    Set other = { 10 };
    set.swap(other);

    AWL_ASSERT(set.contains(10));
    AWL_ASSERT(other.contains(1));
    AWL_ASSERT(other.contains(2));
    AWL_ASSERT(other.contains(3));
    AWL_ASSERT(other.contains(4));

    AWL_ASSERT_EQUAL(size_t(4), recorder.added.size());
    AWL_ASSERT_EQUAL(size_t(1), recorder.removed.size());
    AWL_ASSERT_EQUAL(1, recorder.clearing_count);
}
