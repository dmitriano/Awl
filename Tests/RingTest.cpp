#include "Awl/Ring.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"
#include "Awl/IntRange.h"

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

            typename A::value_type rbegin_val = *a.rbegin();
            AWT_ASSERT(rbegin_val == *b.rbegin());
        }

        AWT_ASSERT(std::equal(a.begin(), a.end(), b.begin(), b.end()));
        AWT_ASSERT(std::equal(a.rbegin(), a.rend(), b.rbegin(), b.rend()));
    }
    
    template <class T>
    class Test
    {
    public:

        Test(std::vector<T> v, size_t cap) : m_v(v), m_r(cap)
        {
            AWT_ASSERT(m_r.capacity() == cap);
            AWT_ASSERT(m_r.size() == 0);

            for (const T & val : v)
            {
                m_r.push_back(val);
            }

            AWT_ASSERT_EQUAL(std::min(m_v.size(), cap), m_r.size());

            m_v.erase(m_v.begin(), m_v.end() - m_r.size());
        }

        void TestContent()
        {
            CompareContainers(m_r, m_v);
            CompareContainers<const decltype(m_r), const decltype(m_v)>(m_r, m_v);
        }

        void RunAll()
        {
            TestContent();
        }

    private:

        std::vector<T> m_v;
        awl::ring<T> m_r;
    };
}

AWT_TEST(RingVectorTest)
{
    AWT_ATTRIBUTE(int, range, 10);
    AWT_ATTRIBUTE(size_t, capacity, 3);

    auto r = awl::make_int_range<int>(0, range);
    
    Test<int> test(std::vector<int>(r.begin(), r.end()), capacity);

    test.RunAll();
}
