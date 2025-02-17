/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Ring.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"
#include "Awl/IntRange.h"
#include "Awl/RangeUtil.h"

#include "Helpers/NonCopyable.h"

#include <deque>
#include <queue>
#include <ranges>

static_assert(awl::range_over<awl::ring<int>, int>);

namespace
{
    template <class A, class B>
    void CompareContainers(A & a, B & b)
    {
        AWL_ASSERT(a.size() == b.size());

        AWL_ASSERT(a.empty() == b.empty());

        if (!a.empty())
        {
            std::size_t size = a.size();

            for (size_t i = 0; i != size; ++i)
            {
                AWL_ASSERT(a[i] == b[i]);
                AWL_ASSERT(a.at(i) == b.at(i));
            }

            AWL_ASSERT(a.front() == b.front());
            AWL_ASSERT(a.back() == b.back());
        }

        AWL_ASSERT(std::equal(a.begin(), a.end(), b.begin(), b.end()));
        AWL_ASSERT(std::equal(a.rbegin(), a.rend(), b.rbegin(), b.rend()));
    }

    template <class T>
    awl::ring<T> MakeRing(std::deque<T> v, size_t cap)
    {
        awl::ring<T> r(cap);

        AWL_ASSERT(r.capacity() == cap);
        AWL_ASSERT(r.size() == 0);

        for (const T & val : v)
        {
            r.push_back(val);
        }

        AWL_ASSERT_EQUAL(std::min(v.size(), cap), r.size());

        return r;
    }
    
    template <class T>
    class Test
    {
    public:

        Test(std::deque<T> v, awl::ring<T> r) : m_v(std::move(v)), m_r(std::move(r))
        {
            m_v.erase(m_v.begin(), m_v.end() - m_r.size());
        }

        void Compare()
        {
            CompareContainers(m_r, m_v);
            CompareContainers<const decltype(m_r), const decltype(m_v)>(m_r, m_v);
        }

        void RunAll()
        {
            Compare();

            do
            {
                m_v.pop_front();
                m_r.pop_front();
                Compare();
            }
            while (!m_r.empty());
        }

    private:

        std::deque<T> m_v;
        awl::ring<T> m_r;
    };

    using A = awl::testing::helpers::NonCopyable;
}

AWL_TEST(RingInt)
{
    AWL_ATTRIBUTE(int, range, 10);
    AWL_ATTRIBUTE(size_t, capacity, 5);

    auto r = awl::make_int_range<int>(0, range);

    std::deque<int> d(r.begin(), r.end());
    awl::ring<int> ring = MakeRing(d, capacity);

    {
        Test<int> test(d, ring);
        test.RunAll();
    }

    awl::ring<int> ring_copy(100);
    ring_copy.push_back(25);
    ring_copy = ring;

    {
        Test<int> test(d, ring_copy);
        test.RunAll();
    }

    ring.reserve(d.size());

    {
        Test<int> test(d, ring);
        test.RunAll();
    }

    ring.reserve(capacity / 2 + 1);

    {
        Test<int> test(d, ring);
        test.RunAll();
    }
}

AWL_TEST(RingMove)
{
    AWL_ATTRIBUTE(int, range, 10);
    AWL_ATTRIBUTE(size_t, capacity, 5);

    {
        auto r = awl::make_int_range<int>(0, range);

        std::deque<A> d;
        std::transform(r.begin(), r.end(), std::back_inserter(d), [](int val) { return A(val); });

        awl::ring<A> ring;
        ring.reserve(capacity + 10);
        std::transform(r.begin(), r.end(), std::back_inserter(ring), [](int val) { return A(val); });
        ring.reserve(capacity);

        awl::ring<A> ring_copy(100);
        ring_copy.push_back(A(25));
        ring_copy = std::move(ring);

        Test<A> test(std::move(d), std::move(ring_copy));
        test.RunAll();
    }

    AWL_ASSERT_EQUAL(0, A::count);
}

AWL_TEST(RingAlgo)
{
    AWL_ATTRIBUTE(int, range, 10);
    AWL_ATTRIBUTE(size_t, capacity, 5);

    {
        awl::ring<A> ring(capacity);

        for (int i : awl::make_count(range))
        {
            ring.push_back(A(i));
        }

        const int first = std::max(0, range - static_cast<int>(capacity));

        AWL_ASSERT(ring.front() == A(first));

        for (int i : awl::make_count(static_cast<int>(ring.size())))
        {
            {
                const auto iter = std::find(ring.begin(), ring.end(), A(first + i));

                AWL_ASSERT(iter != ring.end());

                AWL_ASSERT(iter - ring.begin() == i);
            }

            {
                const auto iter = std::lower_bound(ring.begin(), ring.end(), A(first + i));

                AWL_ASSERT(iter != ring.end());

                AWL_ASSERT(iter - ring.begin() == i);
            }
        }
    }

    AWL_ASSERT_EQUAL(0, A::count);
}

AWL_TEST(RingDestruction)
{
    AWL_ATTRIBUTE(int, range, 10);
    AWL_ATTRIBUTE(size_t, capacity, 5);

    {
        A a1(1);

        A a2 = std::move(a1);
    }

    AWL_ASSERT_EQUAL(0, A::count);

    awl::ring<A> ring(capacity);

    for (int i : awl::make_count(range))
    {
        ring.push_back(A(i));

        //context.out << _T("A::count = ") << A::count << std::endl;
    }

    AWL_ASSERT_EQUAL(std::min(range, static_cast<int>(capacity)), static_cast<int>(ring.size()));
    AWL_ASSERT_EQUAL(static_cast<int>(ring.size()), A::count);

    const size_t new_cap = std::max(static_cast<std::size_t>(1u), ring.size() / 2u);
    ring.reserve(new_cap);

    AWL_ASSERT_EQUAL(static_cast<int>(ring.size()), A::count);

    ring.clear();

    AWL_ASSERT_EQUAL(0, A::count);
}

AWL_TEST(RingQueue)
{
    AWL_ATTRIBUTE(int, range, 10);
    AWL_ATTRIBUTE(size_t, capacity, 5);

    {
        awl::ring<A> ring(capacity);

        std::queue q(std::move(ring));

        for (int i : awl::make_count(range))
        {
            q.push(A(i));
        }

        AWL_ASSERT(q.size() == std::min(static_cast<std::size_t>(range), capacity));

        AWL_ASSERT(q.back() == A(range - 1));

        int i = range - static_cast<int>(q.size());
        
        while (!q.empty())
        {
            AWL_ASSERT(q.front() == A(i++));
            q.pop();
        }
    }

    AWL_ASSERT_EQUAL(0, A::count);
}

AWL_TEST(RingPushFront)
{
    AWL_ATTRIBUTE(int, range, 10);
    AWL_ATTRIBUTE(size_t, capacity, 5);

    awl::ring<A> ring(capacity);

    for (int i : awl::make_count(range))
    {
        ring.push_front(A(i));
    }

    AWL_ASSERT_EQUAL(std::min(range, static_cast<int>(capacity)), static_cast<int>(ring.size()));
    AWL_ASSERT_EQUAL(static_cast<int>(ring.size()), A::count);

    const size_t new_cap = std::max(static_cast<std::size_t>(1u), ring.size() / 2u);
    ring.reserve(new_cap);

    AWL_ASSERT_EQUAL(static_cast<int>(ring.size()), A::count);

    while (!ring.empty())
    {
        ring.pop_back();
    }

    AWL_ASSERT_EQUAL(0, A::count);
}
