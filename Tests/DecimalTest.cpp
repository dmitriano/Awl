#include "Awl/Decimal.h"
#include "Awl/Testing/UnitTest.h"

using namespace std::literals; 

using namespace awl::testing;

namespace
{
    template <class C>
    void TestStringConversion(std::basic_string_view<C> sample, std::basic_string_view<C> result = {})
    {
        awl::decimal d(sample);

        std::basic_ostringstream<C> out;

        out << d;

        auto text = out.str();

        if (result.empty())
        {
            AWT_ASSERT(text == sample);
        }
        else
        {
            AWT_ASSERT(text == result);
        }
    }

    template <class C>
    void CheckTrows(std::basic_string_view<C> sample)
    {
        try
        {
            awl::decimal d(sample);

            AWT_FAILM("It did not throw.");
        }
        catch (const std::exception&)
        {
        }
    }

    std::string to_string(awl::decimal d)
    {
        std::ostringstream out;

        out << d;

        return out.str();
    }
}

AWT_TEST(DecimalStringConversion)
{
    AWT_UNUSED_CONTEXT;

    TestStringConversion("123.456789"sv);
    TestStringConversion(L"123.456789"sv);

    TestStringConversion("-000123.456789"sv, "-123.456789"sv);
    TestStringConversion(L"-000123.456789"sv, L"-123.456789"sv);

    TestStringConversion("+0123.456789"sv, "123.456789"sv);
    TestStringConversion(L"+0123.456789"sv, L"123.456789"sv);

    TestStringConversion("0.456789"sv);
    TestStringConversion(L"0.456789"sv);

    TestStringConversion("0"sv);
    TestStringConversion(L"0"sv);

    TestStringConversion("0.001"sv);
    TestStringConversion(L"0.001"sv);

    TestStringConversion("0.00100"sv, "0.001"sv);
    TestStringConversion(L"0.00100"sv, L"0.001"sv);

    TestStringConversion(".456789"sv, "0.456789"sv);
    TestStringConversion(L".456789"sv, L"0.456789"sv);

    //18 is OK.
    TestStringConversion("0.123456789123456789"sv);
    TestStringConversion(L"0.123456789123456789"sv);

    TestStringConversion("1.12345678912345678"sv);
    TestStringConversion(L"1.12345678912345678"sv);

    TestStringConversion("12.1234567891234567"sv);
    TestStringConversion(L"12.1234567891234567"sv);

    TestStringConversion("121234567891234567"sv);
    TestStringConversion(L"121234567891234567"sv);
}

AWT_TEST(DecimalLimits)
{
    AWT_UNUSED_CONTEXT;

    //19 is wrong
    CheckTrows("0.1234567891234567891"sv);
    CheckTrows(L"0.1234567891234567891"sv);

    CheckTrows("1.123456789123456789"sv);
    CheckTrows(L"1.123456789123456789"sv);

    CheckTrows("12.12345678912345678"sv);
    CheckTrows(L"12.12345678912345678"sv);

    CheckTrows("1212345678912345678"sv);
    CheckTrows(L"1212345678912345678"sv);
}

AWT_TEST(DecimalRescale)
{
    AWT_UNUSED_CONTEXT;

    awl::decimal d("123.45678"sv);

    d.rescale(7);
    
    AWT_ASSERT(to_string(d) == "123.4567800");

    d.rescale(3);

    AWT_ASSERT(to_string(d) == "123.456");

    d.rescale(0);

    AWT_ASSERT(to_string(d) == "123");

    //15 + 3 == 18
    d.rescale(15);

    AWT_ASSERT(to_string(d) == "123.000000000000000");

    d.rescale(0);

    AWT_ASSERT(to_string(d) == "123");

    try
    {
        d.rescale(16);

        AWT_FAILM("It did not throw.");
    }
    catch (const std::exception&)
    {
    }
}

AWT_TEST(DecimalCompare)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(awl::decimal("123.456789"sv) == awl::decimal("123.456789000"sv));
    AWT_ASSERT(awl::decimal("0"sv) == awl::decimal("0"sv));
    AWT_ASSERT(awl::decimal("123"sv) == awl::decimal("123.000"sv));
    AWT_ASSERT(awl::decimal("123"sv) <= awl::decimal("123.000"sv));
    AWT_ASSERT(awl::decimal("123"sv) >= awl::decimal("123.000"sv));

    AWT_ASSERT(awl::decimal("1"sv) != awl::decimal("2.000"sv));

    AWT_ASSERT(awl::decimal("1"sv) < awl::decimal("2.000"sv));
    AWT_ASSERT(awl::decimal("2.000"sv) > awl::decimal("1.000"sv));
    AWT_ASSERT(awl::decimal("1"sv) <= awl::decimal("2.000"sv));
    AWT_ASSERT(awl::decimal("2.000"sv) >= awl::decimal("1.000"sv));
}

AWT_TEST(DecimalCast)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(awl::decimal("1"sv).cast<double>() == 1.0);
    AWT_ASSERT(awl::decimal("1"sv).cast<float>() == 1.0f);
    AWT_ASSERT(awl::decimal("1"sv).cast<int>() == 1);

    AWT_ASSERT(awl::decimal("1"sv) == 1.0);
}

AWT_TEST(DecimalArithmeticOperators)
{
    AWT_UNUSED_CONTEXT;

    //Addition and subtraction
    AWT_ASSERT(awl::decimal("1.0"sv) + awl::decimal("2.000"sv) == awl::decimal("3"sv));
    AWT_ASSERT(awl::decimal("1.05"sv) + awl::decimal("2.005"sv) == awl::decimal("3.055"sv));
    AWT_ASSERT(awl::decimal("1.05"sv) - awl::decimal("0.05"sv) == awl::decimal("1"sv));

    {
        awl::decimal d("1.0"sv);

        d += awl::decimal("2.0"sv);

        AWT_ASSERT(d == awl::decimal("3.0"sv));

        d -= awl::decimal("2.0"sv);

        AWT_ASSERT(d == awl::decimal("1.0"sv));
    }

    //Multiplication and Division
    AWT_ASSERT(awl::make_decimal(awl::decimal("2.0"sv) * awl::decimal("3.000"sv), 5) == awl::decimal("6"sv));
    AWT_ASSERT(awl::make_decimal(awl::decimal("6.0"sv) / awl::decimal("2.000"sv), 5) == awl::decimal("3"sv));
}

AWT_TEST(DecimalMinMax)
{
    AWT_UNUSED_CONTEXT;

    awl::decimal max = std::numeric_limits<awl::decimal>::max();
    awl::decimal min = std::numeric_limits<awl::decimal>::min();

    AWT_ASSERT(max == max);
    AWT_ASSERT(awl::decimal(std::numeric_limits<int64_t>::max() - 1, 0) < max);
    AWT_ASSERT(awl::zero < max);
    AWT_ASSERT(awl::make_decimal(1, awl::decimal::max_digits()) < max);

    AWT_ASSERT(min == min);
    AWT_ASSERT(min < awl::make_decimal(std::numeric_limits<int64_t>::min() + 1, 0));
    AWT_ASSERT(min < awl::zero);
}