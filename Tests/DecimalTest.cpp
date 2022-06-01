/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Decimal.h"
#include "Awl/Testing/UnitTest.h"

using namespace std::literals; 

using namespace awl::testing;
using namespace awl::literals;

#ifdef AWL_DECIMAL_128
    using Decimal = awl::decimal128;
#else
    using Decimal = awl::decimal64;
#endif

namespace
{
    template <class C>
    void TestStringConversion(std::basic_string_view<C> sample, std::basic_string_view<C> result = {})
    {
        Decimal d(sample);

        std::basic_ostringstream<C> out;

        out << d;

        auto fixed_string = out.str();

        if (result.empty())
        {
            AWT_ASSERT(fixed_string == sample);
        }
        else
        {
            AWT_ASSERT(fixed_string == result);
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
            Decimal d(sample);
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

    //17 is OK
    TestStringConversion("12123456789123456"sv);
    TestStringConversion(L"12123456789123456"sv);
}

AWT_TEST(DecimalDoubleConversion)
{
    AWT_UNUSED_CONTEXT;

    const double val = 10.128;

    Decimal d(5);

    d = val;

    const awl::String fixed_string = d.to_string();

    AWT_ASSERT(fixed_string == _T("10.12800"));
}

AWT_TEST(DecimalRescale)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(Decimal::zero().rescale(3).to_astring() == "0.000");

    Decimal d("123.45678"sv);

    d = d.rescale(7);
    
    AWT_ASSERT(d.to_astring() == "123.4567800");

    d = d.rescale(5);

    AWT_ASSERT(d.to_astring() == "123.45678");

    d *= Decimal(100000, 0);
    
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
}

AWT_TEST(DecimalTrim)
{
    AWT_UNUSED_CONTEXT;

    Decimal d("123.45678"sv);

    d = d.truncate(3);

    AWT_ASSERT(d.to_astring() == "123.456");

    d = d.truncate(0);

    AWT_ASSERT(d.to_astring() == "123");
}


AWT_TEST(DecimalCompare)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(Decimal("123.456789"sv) == Decimal("123.456789000"sv));
    AWT_ASSERT(Decimal("0"sv) == Decimal("0"sv));
    AWT_ASSERT(Decimal("123"sv) == Decimal("123.000"sv));
    AWT_ASSERT(Decimal("123"sv) <= Decimal("123.000"sv));
    AWT_ASSERT(Decimal("123"sv) >= Decimal("123.000"sv));

    AWT_ASSERT(Decimal("1"sv) != Decimal("2.000"sv));

    AWT_ASSERT(Decimal("0.5"sv) < Decimal("2"sv));
    AWT_ASSERT(Decimal("0.596"sv) < Decimal("2.232"sv));
    AWT_ASSERT(!(Decimal("0.596"sv) >= Decimal("2.232"sv)));
    AWT_ASSERT(Decimal("1"sv) < Decimal("2.000"sv));
    AWT_ASSERT(Decimal("2.000"sv) > Decimal("1.000"sv));
    AWT_ASSERT(Decimal("1"sv) <= Decimal("2.000"sv));
    AWT_ASSERT(Decimal("2.000"sv) >= Decimal("1.000"sv));

    AWT_ASSERT(Decimal("1"sv) < 2.000);
    AWT_ASSERT(Decimal("2.000"sv) > 1.000);
    AWT_ASSERT(Decimal("1"sv) <= 2.000);
    AWT_ASSERT(Decimal("2.000"sv) >= 1.000);

    //From BZRX/USDT order data
    AWT_ASSERT(Decimal("0.2678"sv) < Decimal("0.3000"sv));
    AWT_ASSERT(Decimal("0.2678"sv) < Decimal("0.3400"sv));
    AWT_ASSERT(Decimal("0.3000"sv) < Decimal("0.3143"sv));
    AWT_ASSERT(Decimal("0.3143"sv) < Decimal("0.3400"sv));
}

AWT_TEST(DecimalCast)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT(Decimal("1"sv).cast<double>() == 1.0);
    AWT_ASSERT(Decimal("1"sv).cast<float>() == 1.0f);
    AWT_ASSERT(Decimal("1"sv).cast<int>() == 1);

    AWT_ASSERT(Decimal("1"sv) == 1.0);
}

AWT_TEST(DecimalArithmeticOperators)
{
    AWT_UNUSED_CONTEXT;

    //Addition and subtraction
    AWT_ASSERT(Decimal("1.0"sv) + Decimal("2.000"sv) == Decimal("3"sv));
    AWT_ASSERT(Decimal("1.05"sv) + Decimal("2.005"sv) == Decimal("3.055"sv));
    AWT_ASSERT(Decimal("1.05"sv) - Decimal("0.05"sv) == Decimal("1"sv));
    AWT_ASSERT((Decimal("1.05"sv) += Decimal("2.005"sv)) == Decimal("3.055"sv));
    AWT_ASSERT((Decimal("1.05"sv) -= Decimal("0.05"sv)) == Decimal("1"sv));
    AWT_ASSERT((Decimal("1"sv) += 2.00) == Decimal("3.0"sv));
    AWT_ASSERT((Decimal("5"sv) -= 2.00) == Decimal("3"sv));

    {
        Decimal d("1.0"sv);

        d += Decimal("2.0"sv);

        AWT_ASSERT(d == Decimal("3.0"sv));

        d -= Decimal("2.0"sv);

        AWT_ASSERT(d == Decimal("1.0"sv));
    }

    //Multiplication and Division
    AWT_ASSERT(Decimal::make_decimal(Decimal("2.0"sv) * Decimal("3.000"sv), 5) == Decimal("6"sv));
    AWT_ASSERT(Decimal::make_decimal(Decimal("6.0"sv) / Decimal("2.000"sv), 5) == Decimal("3"sv));
    AWT_ASSERT((Decimal("2.0"sv) *= Decimal("3.000"sv)) == Decimal("6"sv));
    AWT_ASSERT((Decimal("6.0"sv) /= Decimal("2.000"sv)) == Decimal("3"sv));
    AWT_ASSERT((Decimal("2.0"sv) *= 3.000) == Decimal("6"sv));
    AWT_ASSERT((Decimal("6.0"sv) /= 2.000) == Decimal("3"sv));

    AWT_ASSERT(awl::multiply(Decimal("7"), Decimal("33")) == Decimal("231"));
    //With double it is probably something like 0.23099999999999998
    AWT_ASSERT(awl::multiply(Decimal("0.7"), Decimal("0.33")) == Decimal("0.231"));
}

AWT_TEST(DecimalRound)
{
    AWT_UNUSED_CONTEXT;
    
    {
        const Decimal d33 = Decimal::make_decimal(1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("0.33"));

        const Decimal d66 = Decimal::make_decimal(2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("0.66"));

        const Decimal d67 = Decimal::make_rounded(2.0 / 3.0, 2);
        AWT_ASSERT(d67 == Decimal("0.67"));

        const Decimal d3 = Decimal::make_rounded(3, 1);
        AWT_ASSERT(d3 == Decimal("3"));

        const Decimal d3d = Decimal::make_rounded(3.0, 1);
        AWT_ASSERT(d3d == Decimal("3"));
    }

    {
        const Decimal d33 = Decimal::make_decimal(-1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("-0.33"));

        const Decimal d66 = Decimal::make_decimal(-2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("-0.66"));

        const Decimal d67 = Decimal::make_rounded(-2.0 / 3.0, 2);
        AWT_ASSERT(d67 == Decimal("-0.67"));

        const Decimal d3 = Decimal::make_rounded(-3, 1);
        AWT_ASSERT(d3 == Decimal("-3"));

        const Decimal d3d = Decimal::make_rounded(-3.0, 1);
        AWT_ASSERT(d3d == Decimal("-3"));
    }

    {
        const Decimal d33 = Decimal::make_ceiled(1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("0.34"));

        const Decimal d66 = Decimal::make_ceiled(2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("0.67"));

        const Decimal d3 = Decimal::make_ceiled(3, 1);
        AWT_ASSERT(d3 == Decimal("3"));

        const Decimal d3d = Decimal::make_ceiled(3.0, 1);
        AWT_ASSERT(d3d == Decimal("3"));
    }

    {
        const Decimal d33 = Decimal::make_ceiled(-1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("-0.33"));

        const Decimal d66 = Decimal::make_ceiled(-2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("-0.66"));

        const Decimal d3 = Decimal::make_ceiled(-3, 1);
        AWT_ASSERT(d3 == Decimal("-3"));

        const Decimal d3d = Decimal::make_ceiled(-3.0, 1);
        AWT_ASSERT(d3d == Decimal("-3"));
    }

    {
        const Decimal d33 = Decimal::make_floored(1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("0.33"));

        const Decimal d66 = Decimal::make_floored(2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("0.66"));

        const Decimal d3 = Decimal::make_floored(3, 1);
        AWT_ASSERT(d3 == Decimal("3"));

        const Decimal d3d = Decimal::make_floored(3.0, 1);
        AWT_ASSERT(d3d == Decimal("3"));
    }

    {
        const Decimal d33 = Decimal::make_floored(-1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("-0.34"));

        const Decimal d66 = Decimal::make_floored(-2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("-0.67"));

        const Decimal d3 = Decimal::make_floored(-3, 1);
        AWT_ASSERT(d3 == Decimal("-3"));

        const Decimal d3d = Decimal::make_floored(-3.0, 1);
        AWT_ASSERT(d3d == Decimal("-3"));
    }

    {
        const Decimal d33 = Decimal::make_truncated(1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("0.33"));

        const Decimal d66 = Decimal::make_truncated(2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("0.66"));

        const Decimal d3 = Decimal::make_truncated(3, 1);
        AWT_ASSERT(d3 == Decimal("3"));
    }

    {
        const Decimal d33 = Decimal::make_truncated(-1.0 / 3.0, 2);
        AWT_ASSERT(d33 == Decimal("-0.33"));

        const Decimal d66 = Decimal::make_truncated(-2.0 / 3.0, 2);
        AWT_ASSERT(d66 == Decimal("-0.66"));

        const Decimal d3 = Decimal::make_truncated(-3, 1);
        AWT_ASSERT(d3 == Decimal("-3"));
    }
}

#ifndef AWL_DECIMAL_128

AWT_TEST(DecimalRescaleOverflow)
{
    AWT_UNUSED_CONTEXT;

    Decimal d("12345678.0000000000"sv);

    CheckTrows([&]()
    {
        //TO DO: Why 11?
        d.rescale(11);
    });
}

AWT_TEST(DecimalStringConversionOverflow)
{
    AWT_UNUSED_CONTEXT;

    CheckTrows([&]()
    {
        TestStringConversion("1234.123456789123456"sv);
    });
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

AWT_TEST(DecimalMinMax)
{
    AWT_UNUSED_CONTEXT;

    Decimal max = std::numeric_limits<Decimal>::max();
    Decimal min = std::numeric_limits<Decimal>::min();

    AWT_ASSERT(min < max);
    AWT_ASSERT(max > min);

    AWT_ASSERT(max == max);
    AWT_ASSERT(max - Decimal(1, 0) < max);
    AWT_ASSERT(Decimal(1, 18) < max);
    AWT_ASSERT(Decimal(-1, 18) < max);
    AWT_ASSERT(Decimal::zero() < max);
    AWT_ASSERT(Decimal::make_decimal(1, Decimal::max_exponent()) < max);

    AWT_ASSERT(min == min);
    AWT_ASSERT(min < min + Decimal(1, 0));
    AWT_ASSERT(min < Decimal::zero());

    AWT_ASSERT(min <= max);
    AWT_ASSERT(max >= min);

    AWT_ASSERT(max != max - Decimal(1, 0));
    AWT_ASSERT(max - Decimal(1, 0) <= max);
    AWT_ASSERT(Decimal::zero() <= max);
    AWT_ASSERT(Decimal::make_decimal(1, Decimal::max_exponent()) <= max);

    AWT_ASSERT(min != min + Decimal(1, 0));
    AWT_ASSERT(min <= min + Decimal(1, 0));
    AWT_ASSERT(min <= Decimal::zero());

    for (uint8_t precision = 0; precision <= Decimal::max_exponent(); ++precision)
    {
        AWT_ASSERT(Decimal(1, precision) > min);
        AWT_ASSERT(Decimal(-1, precision) > min);
        AWT_ASSERT(Decimal(1, precision) >= min);
        AWT_ASSERT(Decimal(-1, precision) >= min);

        AWT_ASSERT(Decimal(1, precision) < max);
        AWT_ASSERT(Decimal(-1, precision) < max);
        AWT_ASSERT(Decimal(1, precision) <= max);
        AWT_ASSERT(Decimal(-1, precision) <= max);

        AWT_ASSERT(Decimal(1, precision) > Decimal::zero());
        AWT_ASSERT(Decimal(-1, precision) < Decimal::zero());
        AWT_ASSERT(Decimal(-1, precision) < Decimal(1, precision));
        AWT_ASSERT(Decimal(1, precision) > Decimal(-1, precision));

        AWT_ASSERT(Decimal(1, precision) >= Decimal::zero());
        AWT_ASSERT(Decimal(-1, precision) <= Decimal::zero());
        AWT_ASSERT(Decimal(-1, precision) <= Decimal(1, precision));
        AWT_ASSERT(Decimal(1, precision) >= Decimal(-1, precision));
    }
}

//It is not enough to store some values from Binance.
AWT_EXAMPLE(DecimalBinanceValues)
{
    constexpr uint64_t max_uint = std::numeric_limits<uint64_t>::max();

    context.out << _T("uint64_t max: ") << max_uint << ", decimal digits: " << static_cast<size_t>(std::log10(max_uint)) << std::endl;

    context.out << _T("max decimal man: ") << Decimal::max_mantissa() << _T(", max decimal exp: ") << Decimal::max_exponent() << std::endl;

    CheckConstructorTrows("8426879770.74001051"sv); //BTC day volume

    CheckConstructorTrows("92233720368.54775807"sv); //LUNA max position
}

#endif //AWL_DECIMAL_128
