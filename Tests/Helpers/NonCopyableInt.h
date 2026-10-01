/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace awl::testing::helpers
{
    class NonCopyableInt
    {
    public:

        using value_type = int;

        explicit NonCopyableInt(int a) : _a(a)
        {
            ++count;
        }

        ~NonCopyableInt()
        {
            --count;
        }

        NonCopyableInt(NonCopyableInt const &) = delete;

        NonCopyableInt(NonCopyableInt && other) : NonCopyableInt(other._a)
        {
            other._moved = true;
        }

        NonCopyableInt & operator = (const NonCopyableInt &) = delete;

        NonCopyableInt & operator = (NonCopyableInt && other)
        {
            _a = other._a;
            other._moved = true;

            return *this;
        }

        bool operator == (const NonCopyableInt & other) const
        {
            return _a == other._a;
        }

        bool operator != (const NonCopyableInt & other) const
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

        bool operator < (const NonCopyableInt & other) const
        {
            return _a < other._a;
        }

        static inline int count = 0;

    private:

        bool _moved = false;
        int _a;
    };
}
