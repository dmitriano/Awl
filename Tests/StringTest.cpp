/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/String.h"
#include "Awl/FixedString.h"

#include "Awl/Testing/UnitTest.h"

namespace
{
    template <class C>
    void TestStringCompare()
    {
        awl::CStringInsensitiveLess<C> less;

        auto text = []<std::size_t N>(const char(&arr)[N])
        {
            return awl::fixed_string<C, N - 1>::from_ascii(arr);
        };

        {
            auto left = text("");
            auto right = text("");

            AWT_ASSERT(!less(left.c_str(), right.c_str()));
            AWT_ASSERT(!less(right.c_str(), left.c_str()));
        }

        {
            auto left = text("abc");
            auto right = text("ABC");

            AWT_ASSERT(!less(left.c_str(), right.c_str()));
            AWT_ASSERT(!less(right.c_str(), left.c_str()));
        }

        {
            auto left = text("");
            auto right = text("a");

            AWT_ASSERT(less(left.c_str(), right.c_str()));
            AWT_ASSERT(!less(right.c_str(), left.c_str()));
        }

        {
            auto left = text("ABC");
            auto right = text("abc1");

            AWT_ASSERT(less(left.c_str(), right.c_str()));
            AWT_ASSERT(!less(right.c_str(), left.c_str()));
        }

        {
            auto left = text("ABC1");
            auto right = text("abc2");

            AWT_ASSERT(less(left.c_str(), right.c_str()));
            AWT_ASSERT(!less(right.c_str(), left.c_str()));
        }

        {
            auto left = text("abc1");
            auto right = text("ABC2");

            AWT_ASSERT(less(left.c_str(), right.c_str()));
            AWT_ASSERT(!less(right.c_str(), left.c_str()));
        }

        {
            auto left = text("abc");
            auto right = text("ABC1");

            AWT_ASSERT(less(left.c_str(), right.c_str()));
            AWT_ASSERT(!less(right.c_str(), left.c_str()));
        }
    }
}

AWT_TEST(StringCompare)
{
    AWT_UNUSED_CONTEXT;

    TestStringCompare<char>();
    TestStringCompare<wchar_t>();
}
