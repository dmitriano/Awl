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
    class StringTest
    {
    public:
        
        static void TestLess()
        {
            awl::CStringInsensitiveLess<C> less;

            {
                auto left = text("");
                auto right = text("");

                AWL_ASSERT(!less(left.c_str(), right.c_str()));
                AWL_ASSERT(!less(right.c_str(), left.c_str()));
            }

            {
                auto left = text("abc");
                auto right = text("ABC");

                AWL_ASSERT(!less(left.c_str(), right.c_str()));
                AWL_ASSERT(!less(right.c_str(), left.c_str()));
            }

            {
                auto left = text("");
                auto right = text("a");

                AWL_ASSERT(less(left.c_str(), right.c_str()));
                AWL_ASSERT(!less(right.c_str(), left.c_str()));
            }

            {
                auto left = text("ABC");
                auto right = text("abc1");

                AWL_ASSERT(less(left.c_str(), right.c_str()));
                AWL_ASSERT(!less(right.c_str(), left.c_str()));
            }

            {
                auto left = text("ABC1");
                auto right = text("abc2");

                AWL_ASSERT(less(left.c_str(), right.c_str()));
                AWL_ASSERT(!less(right.c_str(), left.c_str()));
            }

            {
                auto left = text("abc1");
                auto right = text("ABC2");

                AWL_ASSERT(less(left.c_str(), right.c_str()));
                AWL_ASSERT(!less(right.c_str(), left.c_str()));
            }

            {
                auto left = text("abc");
                auto right = text("ABC1");

                AWL_ASSERT(less(left.c_str(), right.c_str()));
                AWL_ASSERT(!less(right.c_str(), left.c_str()));
            }
        }

#ifdef AWL_INT_128

        static void TestInt128Format()
        {
            const uint64_t max = std::numeric_limits<uint64_t>::max();

            std::basic_string<C> max_text = FormatInt(max);

            const __uint128_t val = static_cast<__uint128_t>(max) * 10 + 3;

            const std::basic_string<C> val_text = FormatInt(val);

            const std::basic_string<C> last_digit_text = text("3");

            AWL_ASSERT(val_text == max_text + last_digit_text);

            // Test a negative value.

            const __int128_t negative_val = -static_cast<__int128_t>(val);

            const std::basic_string<C> negative_val_text = FormatInt(negative_val);

            const std::basic_string<C> minus_text = text("-");

            AWL_ASSERT(negative_val_text == minus_text + val_text);

            // Test zero values.

            AssertEqual(static_cast<__uint128_t>(0), text("0"));
            AssertEqual(static_cast<__int128_t>(0), text("0"));

            // Test fill and width.
            
            using awl::operator <<;
            
            std::basic_ostringstream<C> out;

            out << std::setfill(static_cast<C>('0')) << std::setw(val_text.length() + 1);

            out << val;

            const std::basic_string<C> val_text_with_prefix = out.str();

            const std::basic_string<C> prefix_text = text("0");

            AWL_ASSERT(val_text_with_prefix == prefix_text + val_text);
        }

#endif //AWL_INT_128

    private:

        template <class Int>
        static std::basic_string<C> FormatInt(Int val)
        {
            using awl::operator <<;
            
            std::basic_ostringstream<C> out;

            out << val;

            return out.str();
        }
        
        template <class Int>
        static void AssertEqual(Int val, const std::basic_string<C>& expected_text)
        {
            std::basic_string<C> val_text = FormatInt(val);

            AWL_ASSERT(val_text == expected_text);
        }
        
        template <std::size_t N>
        static auto text(const char(&arr)[N])
        {
            return awl::fixed_string<C, N - 1>::from_ascii(arr);
        }
    };
}

AWL_TEST(StringCompare)
{
    AWL_UNUSED_CONTEXT;

    StringTest<char>::TestLess();
    StringTest<wchar_t>::TestLess();
}

#ifdef AWL_INT_128

AWL_TEST(StringInt128)
{
    AWL_UNUSED_CONTEXT;

    StringTest<char>::TestInt128Format();
    StringTest<wchar_t>::TestInt128Format();
}

#endif //AWL_INT_128
