#include "Awl/Ring.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

namespace
{
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
        }

        void TestContent()
        {
            for (size_t i = 0; i != m_r.size(); ++i)
            {
                AWT_ASSERT(m_r[i] == m_v[m_v.size() - 1 - m_r.size() + i]);
            }
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
    AWT_ATTRIBUTE(size_t, capacity, 3);

    Test<int> test({1, 2, 3, 4, 5}, capacity);

    test.RunAll();
}
