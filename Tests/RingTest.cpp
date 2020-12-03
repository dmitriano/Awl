#include "Awl/Ring.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"
#include "Awl/IntRange.h"

#include <deque>
#include <queue>
#include <algorithm>

namespace
{
    template <class A, class B>
    void CompareContainers(A & a, B & b)
    {
        AWT_ASSERT(a.size() == b.size());

        AWT_ASSERT(a.empty() == b.empty());

        if (!a.empty())
        {
            std::size_t size = a.size();

            for (size_t i = 0; i != size; ++i)
            {
                AWT_ASSERT(a[i] == b[i]);
                AWT_ASSERT(a.at(i) == b.at(i));
            }

            AWT_ASSERT(a.front() == b.front());
            AWT_ASSERT(a.back() == b.back());
        }

        AWT_ASSERT(std::equal(a.begin(), a.end(), b.begin(), b.end()));
        AWT_ASSERT(std::equal(a.rbegin(), a.rend(), b.rbegin(), b.rend()));
    }

    template <class T>
    awl::ring<T> MakeRing(std::deque<T> v, size_t cap)
    {
        awl::ring<T> r(cap);

        AWT_ASSERT(r.capacity() == cap);
        AWT_ASSERT(r.size() == 0);

        for (const T & val : v)
        {
            r.push_back(val);
        }

        AWT_ASSERT_EQUAL(std::min(v.size(), cap), r.size());

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

    class A
    {
    public:

        explicit A(int a) : m_a(a)
        {
            ++count;
        }

        ~A()
        {
            --count;
        }

        A(A const &) = delete;

        A(A && other) : A(other.m_a)
        {
            other.m_moved = true;
        }

        A & operator = (const A &) = delete;

        A & operator = (A && other)
        {
            m_a = other.m_a;
            other.m_moved = true;

            return *this;
        }

        bool operator == (const A & other) const
        {
            return m_a == other.m_a;
        }

        bool operator != (const A & other) const
        {
            return !operator==(other);
        }

        bool operator < (const A & other) const
        {
            return m_a < other.m_a;
        }

        static int count;

    private:


        bool m_moved = false;
        int m_a;
    };

    int A::count = 0;
}

AWT_TEST(RingIntTest)
{
    AWT_ATTRIBUTE(int, range, 10);
    AWT_ATTRIBUTE(size_t, capacity, 5);

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

AWT_TEST(RingMoveTest)
{
    AWT_ATTRIBUTE(int, range, 10);
    AWT_ATTRIBUTE(size_t, capacity, 5);

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

    AWT_ASSERT_EQUAL(0, A::count);
}

AWT_TEST(RingAlgoTest)
{
    AWT_ATTRIBUTE(int, range, 10);
    AWT_ATTRIBUTE(size_t, capacity, 5);

    {
        awl::ring<A> ring(capacity);

        for (int i : awl::make_count(range))
        {
            ring.push_back(A(i));
        }

        const int first = std::max(0, range - static_cast<int>(capacity));

        AWT_ASSERT(ring.front() == A(first));

        for (int i : awl::make_count(static_cast<int>(ring.size())))
        {
            {
                const auto iter = std::find(ring.begin(), ring.end(), A(first + i));

                AWT_ASSERT(iter != ring.end());

                AWT_ASSERT(iter - ring.begin() == i);
            }

            {
                const auto iter = std::lower_bound(ring.begin(), ring.end(), A(first + i));

                AWT_ASSERT(iter != ring.end());

                AWT_ASSERT(iter - ring.begin() == i);
            }
        }
    }

    AWT_ASSERT_EQUAL(0, A::count);
}

AWT_TEST(RingDestructionTest)
{
    AWT_ATTRIBUTE(int, range, 10);
    AWT_ATTRIBUTE(size_t, capacity, 5);

    {
        A a1(1);

        A a2 = std::move(a1);
    }

    AWT_ASSERT_EQUAL(0, A::count);

    awl::ring<A> ring(capacity);

    for (int i : awl::make_count(range))
    {
        ring.push_back(A(i));

        //context.out << _T("A::count = ") << A::count << std::endl;
    }

    AWT_ASSERT_EQUAL(std::min(range, static_cast<int>(capacity)), static_cast<int>(ring.size()));
    AWT_ASSERT_EQUAL(static_cast<int>(ring.size()), A::count);

    const size_t new_cap = std::max(static_cast<std::size_t>(1u), ring.size() / 2u);
    ring.reserve(new_cap);

    AWT_ASSERT_EQUAL(static_cast<int>(ring.size()), A::count);

    ring.clear();

    AWT_ASSERT_EQUAL(0, A::count);
}

AWT_TEST(RingQueueTest)
{
    AWT_ATTRIBUTE(int, range, 10);
    AWT_ATTRIBUTE(size_t, capacity, 5);

    {
        awl::ring<A> ring(capacity);

        std::queue q(std::move(ring));

        for (int i : awl::make_count(range))
        {
            q.push(A(i));
        }

        AWT_ASSERT(q.size() == std::min(static_cast<std::size_t>(range), capacity));

        AWT_ASSERT(q.back() == A(range - 1));

        int i = range - static_cast<int>(q.size());
        
        while (!q.empty())
        {
            AWT_ASSERT(q.front() == A(i++));
            q.pop();
        }
    }

    AWT_ASSERT_EQUAL(0, A::count);
}
