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

        explicit NonCopyable(int a) : _a(a)
        {
            ++count;
        }

        ~NonCopyable()
        {
            --count;
        }

        NonCopyable(NonCopyable const &) = delete;

        NonCopyable(NonCopyable && other) : NonCopyable(other._a)
        {
            other._moved = true;
        }

        NonCopyable & operator = (const NonCopyable &) = delete;

        NonCopyable & operator = (NonCopyable && other)
        {
            _a = other._a;
            other._moved = true;

            return *this;
        }

        bool operator == (const NonCopyable & other) const
        {
            return _a == other._a;
        }

        bool operator != (const NonCopyable & other) const
        {
            return !operator==(other);
        }

        bool operator == (int a) const
        {
            return _a == a;
        }

        bool operator != (int a) const
        {
            return !operator==(a);
        }

        bool operator < (const NonCopyable & other) const
        {
            return _a < other._a;
        }

        static inline int count = 0;

    private:

        bool _moved = false;
        int _a;
    };
}
