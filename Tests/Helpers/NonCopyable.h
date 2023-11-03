/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace awl::testing::helpers
{
    class NonCopyable
    {
    public:

        using value_type = int;

        explicit NonCopyable(int a) : m_a(a)
        {
            ++count;
        }

        ~NonCopyable()
        {
            --count;
        }

        NonCopyable(NonCopyable const &) = delete;

        NonCopyable(NonCopyable && other) : NonCopyable(other.m_a)
        {
            other.m_moved = true;
        }

        NonCopyable & operator = (const NonCopyable &) = delete;

        NonCopyable & operator = (NonCopyable && other)
        {
            m_a = other.m_a;
            other.m_moved = true;

            return *this;
        }

        bool operator == (const NonCopyable & other) const
        {
            return m_a == other.m_a;
        }

        bool operator != (const NonCopyable & other) const
        {
            return !operator==(other);
        }

        bool operator == (int a) const
        {
            return m_a == a;
        }

        bool operator != (int a) const
        {
            return !operator==(a);
        }

        bool operator < (const NonCopyable & other) const
        {
            return m_a < other.m_a;
        }

        static inline int count = 0;

    private:

        bool m_moved = false;
        int m_a;
    };
}
