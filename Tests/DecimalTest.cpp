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

    template <class Func>
    void CheckTrows(Func func)
    {
        bool success = false;

        try
        {
            func();

            success = true;
        }
        catch (const std::exception&)
        {
        }

        if (success)
        {
            //This should be outside of try/catch block, because it also throws a test exception.
            AWT_FAILM("Decimal did not throw.");
        }
    }

    template <class C>
    void CheckConstructorTrows(std::basic_string_view<C> sample)
    {
        CheckTrows([&]()
        {
            awl::decimal d(sample);
        });
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

    //15 is OK.
    TestStringConversion("0.123456789123456"sv);
    TestStringConversion(L"0.123456789123456"sv);

    TestStringConversion("1.123456789123456"sv);
    TestStringConversion(L"1.123456789123456"sv);

    TestStringConversion("12.123456789123456"sv);
    TestStringConversion(L"12.123456789123456"sv);

    CheckTrows([&]()
    {
        TestStringConversion("1234.123456789123456"sv);
    });

    //17 is OK
    TestStringConversion("12123456789123456"sv);
    TestStringConversion(L"12123456789123456"sv);
}

AWT_TEST(DecimalDoubleConversion)
{
    AWT_UNUSED_CONTEXT;

    const double val = 10.128;

    awl::decimal d(5);

    d = val;

    const awl::String text = d.to_string();

    AWT_ASSERT(text == _T("10.12800"));
}

AWT_TEST(DecimalLimits)
{
    AWT_UNUSED_CONTEXT;

    //19 is wrong
    CheckConstructorTrows("0.1234567891234567891"sv);
    CheckConstructorTrows(L"0.1234567891234567891"sv);

    CheckConstructorTrows("1.123456789123456789"sv);
    CheckConstructorTrows(L"1.123456789123456789"sv);

    CheckConstructorTrows("12.12345678912345678"sv);
    CheckConstructorTrows(L"12.12345678912345678"sv);

    CheckConstructorTrows("1212345678912345678"sv);
    CheckConstructorTrows(L"1212345678912345678"sv);
}

AWT_TEST(DecimalRescale)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(awl::zero.rescale(3).to_astring() == "0.000");

    awl::decimal d("123.45678"sv);

    d = d.rescale(7);
    
    AWT_ASSERT(d.to_astring() == "123.4567800");

    d = d.rescale(5);

    AWT_ASSERT(d.to_astring() == "123.45678");

    d *= awl::decimal(100000, 0);
    
    d = d.rescale(0);

    AWT_ASSERT(d.to_astring() == "12345678");

    //9 + 8 = 17
    d = d.rescale(9);

    AWT_ASSERT(d.to_astring() == "12345678.000000000");

    d = d.rescale(0);

    AWT_ASSERT(d.to_astring() == "12345678");

    //TO DO: Why 10 works?
    d = d.rescale(10);

    AWT_ASSERT(d.to_astring() == "12345678.0000000000");

    CheckTrows([&]()
    {
        //TO DO: Why 11?
        d.rescale(11);
    });
}

AWT_TEST(DecimalTrim)
{
    AWT_UNUSED_CONTEXT;

    awl::decimal d("123.45678"sv);

    d = d.trim(3);

    AWT_ASSERT(d.to_astring() == "123.456");

    d = d.trim(0);

    AWT_ASSERT(d.to_astring() == "123");
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

    AWT_ASSERT(awl::decimal("0.5"sv) < awl::decimal("2"sv));
    AWT_ASSERT(awl::decimal("0.596"sv) < awl::decimal("2.232"sv));
    AWT_ASSERT(!(awl::decimal("0.596"sv) >= awl::decimal("2.232"sv)));
    AWT_ASSERT(awl::decimal("1"sv) < awl::decimal("2.000"sv));
    AWT_ASSERT(awl::decimal("2.000"sv) > awl::decimal("1.000"sv));
    AWT_ASSERT(awl::decimal("1"sv) <= awl::decimal("2.000"sv));
    AWT_ASSERT(awl::decimal("2.000"sv) >= awl::decimal("1.000"sv));

    AWT_ASSERT(awl::decimal("1"sv) < 2.000);
    AWT_ASSERT(awl::decimal("2.000"sv) > 1.000);
    AWT_ASSERT(awl::decimal("1"sv) <= 2.000);
    AWT_ASSERT(awl::decimal("2.000"sv) >= 1.000);

    //From BZRX/USDT order data
    AWT_ASSERT(awl::decimal("0.2678"sv) < awl::decimal("0.3000"sv));
    AWT_ASSERT(awl::decimal("0.2678"sv) < awl::decimal("0.3400"sv));
    AWT_ASSERT(awl::decimal("0.3000"sv) < awl::decimal("0.3143"sv));
    AWT_ASSERT(awl::decimal("0.3143"sv) < awl::decimal("0.3400"sv));
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
    AWT_ASSERT((awl::decimal("1.05"sv) += awl::decimal("2.005"sv)) == awl::decimal("3.055"sv));
    AWT_ASSERT((awl::decimal("1.05"sv) -= awl::decimal("0.05"sv)) == awl::decimal("1"sv));
    AWT_ASSERT((awl::decimal("1"sv) += 2.00) == awl::decimal("3.0"sv));
    AWT_ASSERT((awl::decimal("5"sv) -= 2.00) == awl::decimal("3"sv));

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
    AWT_ASSERT((awl::decimal("2.0"sv) *= awl::decimal("3.000"sv)) == awl::decimal("6"sv));
    AWT_ASSERT((awl::decimal("6.0"sv) /= awl::decimal("2.000"sv)) == awl::decimal("3"sv));
    AWT_ASSERT((awl::decimal("2.0"sv) *= 3.000) == awl::decimal("6"sv));
    AWT_ASSERT((awl::decimal("6.0"sv) /= 2.000) == awl::decimal("3"sv));
}

AWT_TEST(DecimalMinMax)
{
    AWT_UNUSED_CONTEXT;

    awl::decimal max = std::numeric_limits<awl::decimal>::max();
    awl::decimal min = std::numeric_limits<awl::decimal>::min();

    AWT_ASSERT(min < max);
    AWT_ASSERT(max > min);

    AWT_ASSERT(max == max);
    AWT_ASSERT(max - awl::decimal(1, 0) < max);
    AWT_ASSERT(awl::decimal(1, 18) < max);
    AWT_ASSERT(awl::decimal(-1, 18) < max);
    AWT_ASSERT(awl::zero < max);
    AWT_ASSERT(awl::make_decimal(1, awl::decimal::max_exponent()) < max);

    AWT_ASSERT(min == min);
    AWT_ASSERT(min < min + awl::decimal(1, 0));
    AWT_ASSERT(min < awl::zero);

    AWT_ASSERT(min <= max);
    AWT_ASSERT(max >= min);

    AWT_ASSERT(max != max - awl::decimal(1, 0));
    AWT_ASSERT(max - awl::decimal(1, 0) <= max);
    AWT_ASSERT(awl::zero <= max);
    AWT_ASSERT(awl::make_decimal(1, awl::decimal::max_exponent()) <= max);

    AWT_ASSERT(min != min + awl::decimal(1, 0));
    AWT_ASSERT(min <= min + awl::decimal(1, 0));
    AWT_ASSERT(min <= awl::zero);

    for (uint8_t precision = 0; precision <= awl::decimal::max_exponent(); ++precision)
    {
        AWT_ASSERT(awl::decimal(1, precision) > min);
        AWT_ASSERT(awl::decimal(-1, precision) > min);
        AWT_ASSERT(awl::decimal(1, precision) >= min);
        AWT_ASSERT(awl::decimal(-1, precision) >= min);

        AWT_ASSERT(awl::decimal(1, precision) < max);
        AWT_ASSERT(awl::decimal(-1, precision) < max);
        AWT_ASSERT(awl::decimal(1, precision) <= max);
        AWT_ASSERT(awl::decimal(-1, precision) <= max);

        AWT_ASSERT(awl::decimal(1, precision) > awl::zero);
        AWT_ASSERT(awl::decimal(-1, precision) < awl::zero);
        AWT_ASSERT(awl::decimal(-1, precision) < awl::decimal(1, precision));
        AWT_ASSERT(awl::decimal(1, precision) > awl::decimal(-1, precision));

        AWT_ASSERT(awl::decimal(1, precision) >= awl::zero);
        AWT_ASSERT(awl::decimal(-1, precision) <= awl::zero);
        AWT_ASSERT(awl::decimal(-1, precision) <= awl::decimal(1, precision));
        AWT_ASSERT(awl::decimal(1, precision) >= awl::decimal(-1, precision));
    }
}

AWT_TEST(DecimalRound)
{
    AWT_UNUSED_CONTEXT;
    
    {
        const awl::decimal d33 = awl::make_decimal(1.0 / 3.0, 2);
        AWT_ASSERT(d33 == awl::decimal("0.33"));

        const awl::decimal d66 = awl::make_decimal(2.0 / 3.0, 2);
        AWT_ASSERT(d66 == awl::decimal("0.66"));

        const awl::decimal d67 = awl::make_rounded(2.0 / 3.0, 2);
        AWT_ASSERT(d67 == awl::decimal("0.67"));

        const awl::decimal d3 = awl::make_rounded(3, 1);
        AWT_ASSERT(d3 == awl::decimal("0.3"));
    }

    {
        const awl::decimal d33 = awl::make_decimal(-1.0 / 3.0, 2);
        AWT_ASSERT(d33 == awl::decimal("-0.33"));

        const awl::decimal d66 = awl::make_decimal(-2.0 / 3.0, 2);
        AWT_ASSERT(d66 == awl::decimal("-0.66"));

        const awl::decimal d67 = awl::make_rounded(-2.0 / 3.0, 2);
        AWT_ASSERT(d67 == awl::decimal("-0.67"));

        const awl::decimal d3 = awl::make_rounded(-3, 1);
        AWT_ASSERT(d3 == awl::decimal("-0.3"));
    }

    {
        const awl::decimal d33 = awl::make_ceiled(1.0 / 3.0, 2);
        AWT_ASSERT(d33 == awl::decimal("0.34"));

        const awl::decimal d66 = awl::make_ceiled(2.0 / 3.0, 2);
        AWT_ASSERT(d66 == awl::decimal("0.67"));

        const awl::decimal d3 = awl::make_ceiled(3, 1);
        AWT_ASSERT(d3 == awl::decimal("0.3"));
    }

    {
        const awl::decimal d33 = awl::make_ceiled(-1.0 / 3.0, 2);
        AWT_ASSERT(d33 == awl::decimal("-0.33"));

        const awl::decimal d66 = awl::make_ceiled(-2.0 / 3.0, 2);
        AWT_ASSERT(d66 == awl::decimal("-0.66"));

        const awl::decimal d3 = awl::make_ceiled(-3, 1);
        AWT_ASSERT(d3 == awl::decimal("-0.3"));
    }

    {
        const awl::decimal d33 = awl::make_floored(1.0 / 3.0, 2);
        AWT_ASSERT(d33 == awl::decimal("0.33"));

        const awl::decimal d66 = awl::make_floored(2.0 / 3.0, 2);
        AWT_ASSERT(d66 == awl::decimal("0.66"));

        const awl::decimal d3 = awl::make_floored(3, 1);
        AWT_ASSERT(d3 == awl::decimal("0.3"));
    }

    {
        const awl::decimal d33 = awl::make_floored(-1.0 / 3.0, 2);
        AWT_ASSERT(d33 == awl::decimal("-0.34"));

        const awl::decimal d66 = awl::make_floored(-2.0 / 3.0, 2);
        AWT_ASSERT(d66 == awl::decimal("-0.67"));

        const awl::decimal d3 = awl::make_floored(-3, 1);
        AWT_ASSERT(d3 == awl::decimal("-0.3"));
    }
}