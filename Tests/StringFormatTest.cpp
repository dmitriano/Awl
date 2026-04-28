/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/StringFormat.h"
#include "Awl/OptionalFormatter.h"

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

        static void TestEqual()
        {
            awl::CStringInsensitiveEqual<C> equal;

            {
                auto left = text("");
                auto right = text("");

                AWL_ASSERT(equal(left.c_str(), right.c_str()));
            }

            {
                auto left = text("abc");
                auto right = text("ABC");

                AWL_ASSERT(equal(left.c_str(), right.c_str()));
                AWL_ASSERT(equal(right.c_str(), left.c_str()));
            }

            {
                auto left = text("");
                auto right = text("a");

                AWL_ASSERT(!equal(left.c_str(), right.c_str()));
                AWL_ASSERT(!equal(right.c_str(), left.c_str()));
            }

            {
                auto left = text("ABC");
                auto right = text("abc1");

                AWL_ASSERT(!equal(left.c_str(), right.c_str()));
                AWL_ASSERT(!equal(right.c_str(), left.c_str()));
            }

            {
                auto left = text("ABC1");
                auto right = text("abc2");

                AWL_ASSERT(!equal(left.c_str(), right.c_str()));
                AWL_ASSERT(!equal(right.c_str(), left.c_str()));
            }

            {
                auto left = text("abc1");
                auto right = text("ABC2");

                AWL_ASSERT(!equal(left.c_str(), right.c_str()));
                AWL_ASSERT(!equal(right.c_str(), left.c_str()));
            }

            {
                auto left = text("abc");
                auto right = text("ABC1");

                AWL_ASSERT(!equal(left.c_str(), right.c_str()));
                AWL_ASSERT(!equal(right.c_str(), left.c_str()));
            }
        }

        static void TestCrossCharStdFormat()
        {
            const std::wstring wide_from_string = std::format(L"{}", std::string("abc"));
            AWL_ASSERT(wide_from_string == std::wstring(L"abc"));

            const std::string narrow_from_wstring = std::format("{}", std::wstring(L"abc"));
            AWL_ASSERT(narrow_from_wstring == std::string("abc"));

#ifdef AWL_QT

            const QString qtext("abc");

            const std::wstring wide_from_qstring = std::format(L"{}", qtext);
            AWL_ASSERT(wide_from_qstring == std::wstring(L"abc"));

            const std::string narrow_from_qstring = std::format("{}", qtext);
            AWL_ASSERT(narrow_from_qstring == std::string("abc"));

#endif
        }

        static void TestOptionalStdFormat()
        {
            const std::optional<int> int_val = 7;
            AWL_ASSERT(std::format("{}", int_val) == std::string("7"));
            AWL_ASSERT(std::format(L"{}", int_val) == std::wstring(L"7"));

            const std::optional<int> empty_int;
            AWL_ASSERT(std::format("{}", empty_int) == std::string("null"));
            AWL_ASSERT(std::format(L"{}", empty_int) == std::wstring(L"null"));
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
        static std::basic_string<C> text(const char(&arr)[N])
        {
            return awl::string_from_ascii<C>(arr);
        }
    };
}

AWL_TEST(StringLess)
{
    AWL_UNUSED_CONTEXT;

    StringTest<char>::TestLess();
    StringTest<wchar_t>::TestLess();
}

AWL_TEST(StringEqual)
{
    AWL_UNUSED_CONTEXT;

    StringTest<char>::TestEqual();
    StringTest<wchar_t>::TestEqual();
}

AWL_TEST(StringCrossCharStdFormat)
{
    AWL_UNUSED_CONTEXT;

    StringTest<char>::TestCrossCharStdFormat();
    StringTest<wchar_t>::TestCrossCharStdFormat();
}

AWL_TEST(StringOptionalStdFormat)
{
    AWL_UNUSED_CONTEXT;

    StringTest<char>::TestOptionalStdFormat();
    StringTest<wchar_t>::TestOptionalStdFormat();
}

#ifdef AWL_INT_128

AWL_TEST(StringInt128)
{
    AWL_UNUSED_CONTEXT;

    StringTest<char>::TestInt128Format();
    StringTest<wchar_t>::TestInt128Format();
}

#endif //AWL_INT_128
