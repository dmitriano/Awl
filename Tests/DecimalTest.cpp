/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Decimal.h"
#include "Awl/Testing/UnitTest.h"

#ifdef AWL_BOOST
#include "BoostExtras/DecimalData.h"
#endif

#include <functional>

#if defined(__GNUC__) || defined(__clang__)
#define AWL_DECIMAL_128 1
#endif

#ifdef AWL_DECIMAL_128

template <class C>
std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, __uint128_t val)
{
    //Obviously not the real implementation, just here to make the tests work
    return out << static_cast<uint64_t>(val);
}

#define GCC_SECTION(test_name) \
    { Test<awl::decimal<__uint128_t, 4>> test; test.test_name(); } \
    { Test<awl::decimal<__uint128_t, 5>> test; test.test_name(); } \
    { Test<awl::decimal<__uint128_t, 6>> test; test.test_name(); }

#else

#define GCC_SECTION(test_name)

#endif

#define LOCAL_TEST(test_name) AWT_TEST(test_name) \
{ \
    AWT_UNUSED_CONTEXT; \
    { Test<awl::decimal<uint64_t, 4>> test; test.test_name(); } \
    { Test<awl::decimal<uint64_t, 5>> test; test.test_name(); } \
    { Test<awl::decimal<uint64_t, 6>> test; test.test_name(); } \
    GCC_SECTION(test_name) \
}

using namespace std::literals;

using namespace awl::testing;

namespace
{
    using Decimal64 = awl::decimal<uint64_t, 4>;

#ifdef AWL_DECIMAL_128

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, __uint128_t val)
    {
        //Obviously not the real implementation, just here as an example
        return out << static_cast<uint64_t>(val);
    }

    using Decimal128 = awl::decimal<__uint128_t, 4>;

#endif

    //constexpr Decimal operator"" _d(const char* str, std::size_t len)
    //{
    //    return Decimal(std::string_view(str, len));
    //}

    //constexpr Decimal operator"" _d(const wchar_t* str, std::size_t len)
    //{
    //    return Decimal(std::wstring_view(str, len));
    //}

    template <class Decimal>
    class Test
    {

    public:

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

        void DecimalStringConversion()
        {
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

        void DecimalDoubleConversion()
        {
            const double val = 10.128;

            Decimal d(5);

            d = val;

            const awl::String fixed_string = d.to_string();

            AWT_ASSERT(fixed_string == _T("10.12800"));
        }

        void DecimalRescale()
        {
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

        void DecimalTrim()
        {
            Decimal d("123.45678"sv);

            d = d.truncate(3);

            AWT_ASSERT(d.to_astring() == "123.456");

            d = d.truncate(0);

            AWT_ASSERT(d.to_astring() == "123");
        }


        void DecimalCompare()
        {
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

        void DecimalCast()
        {
            AWT_ASSERT(Decimal("1"sv).cast<double>() == 1.0);
            AWT_ASSERT(Decimal("1"sv).cast<float>() == 1.0f);
            AWT_ASSERT(Decimal("1"sv).cast<int>() == 1);

            AWT_ASSERT(Decimal("1"sv) == 1.0);
        }

        void DecimalArithmeticOperators()
        {
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

        void DecimalRound()
        {
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
    };
}

LOCAL_TEST(DecimalStringConversion)

LOCAL_TEST(DecimalRescale)

LOCAL_TEST(DecimalCompare)

LOCAL_TEST(DecimalCast)

LOCAL_TEST(DecimalArithmeticOperators)

LOCAL_TEST(DecimalRound)

#ifndef AWL_DECIMAL_128

AWT_TEST(DecimalRescaleOverflow)
{
    AWT_UNUSED_CONTEXT;

    Test<Decimal64> test;

    Decimal64 d("12345678.0000000000"sv);

    test.CheckTrows([&]()
    {
        //TO DO: Why 11?
        d.rescale(11);
    });
}

AWT_TEST(DecimalStringConversionOverflow)
{
    AWT_UNUSED_CONTEXT;

    Test<Decimal64> test;

    test.CheckTrows([&]()
    {
        test.TestStringConversion("1234.123456789123456"sv);
    });
}

AWT_TEST(DecimalLimits)
{
    AWT_UNUSED_CONTEXT;

    Test<Decimal64> test;

    //19 is wrong
    test.CheckConstructorTrows("0.1234567891234567891"sv);
    test.CheckConstructorTrows(L"0.1234567891234567891"sv);

    test.CheckConstructorTrows("1.123456789123456789"sv);
    test.CheckConstructorTrows(L"1.123456789123456789"sv);

    test.CheckConstructorTrows("12.12345678912345678"sv);
    test.CheckConstructorTrows(L"12.12345678912345678"sv);

    test.CheckConstructorTrows("1212345678912345678"sv);
    test.CheckConstructorTrows(L"1212345678912345678"sv);
}

AWT_TEST(DecimalMinMax)
{
    AWT_UNUSED_CONTEXT;

    Decimal64 max = std::numeric_limits<Decimal64>::max();
    Decimal64 min = std::numeric_limits<Decimal64>::min();

    AWT_ASSERT(min < max);
    AWT_ASSERT(max > min);

    AWT_ASSERT(max == max);
    AWT_ASSERT(max - Decimal64(1, 0) < max);
    AWT_ASSERT(Decimal64(1, 18) < max);
    AWT_ASSERT(Decimal64(-1, 18) < max);
    AWT_ASSERT(Decimal64::zero() < max);
    AWT_ASSERT(Decimal64::make_decimal(1, Decimal64::max_exponent()) < max);

    AWT_ASSERT(min == min);
    AWT_ASSERT(min < min + Decimal64(1, 0));
    AWT_ASSERT(min < Decimal64::zero());

    AWT_ASSERT(min <= max);
    AWT_ASSERT(max >= min);

    AWT_ASSERT(max != max - Decimal64(1, 0));
    AWT_ASSERT(max - Decimal64(1, 0) <= max);
    AWT_ASSERT(Decimal64::zero() <= max);
    AWT_ASSERT(Decimal64::make_decimal(1, Decimal64::max_exponent()) <= max);

    AWT_ASSERT(min != min + Decimal64(1, 0));
    AWT_ASSERT(min <= min + Decimal64(1, 0));
    AWT_ASSERT(min <= Decimal64::zero());

    for (uint8_t precision = 0; precision <= Decimal64::max_exponent(); ++precision)
    {
        AWT_ASSERT(Decimal64(1, precision) > min);
        AWT_ASSERT(Decimal64(-1, precision) > min);
        AWT_ASSERT(Decimal64(1, precision) >= min);
        AWT_ASSERT(Decimal64(-1, precision) >= min);

        AWT_ASSERT(Decimal64(1, precision) < max);
        AWT_ASSERT(Decimal64(-1, precision) < max);
        AWT_ASSERT(Decimal64(1, precision) <= max);
        AWT_ASSERT(Decimal64(-1, precision) <= max);

        AWT_ASSERT(Decimal64(1, precision) > Decimal64::zero());
        AWT_ASSERT(Decimal64(-1, precision) < Decimal64::zero());
        AWT_ASSERT(Decimal64(-1, precision) < Decimal64(1, precision));
        AWT_ASSERT(Decimal64(1, precision) > Decimal64(-1, precision));

        AWT_ASSERT(Decimal64(1, precision) >= Decimal64::zero());
        AWT_ASSERT(Decimal64(-1, precision) <= Decimal64::zero());
        AWT_ASSERT(Decimal64(-1, precision) <= Decimal64(1, precision));
        AWT_ASSERT(Decimal64(1, precision) >= Decimal64(-1, precision));
    }
}

//It is not enough to store some values from Binance.
AWT_EXAMPLE(DecimalBinanceValues)
{
    Test<Decimal64> test;
    
    constexpr uint64_t max_uint = std::numeric_limits<uint64_t>::max();

    context.out << _T("uint64_t max: ") << max_uint << ", decimal digits: " << static_cast<size_t>(std::log10(max_uint)) << std::endl;

    context.out << _T("max decimal man: ") << Decimal64::max_mantissa() << _T(", max decimal exp: ") << Decimal64::max_exponent() << std::endl;

    test.CheckConstructorTrows("8426879770.74001051"sv); //BTC day volume

    test.CheckConstructorTrows("92233720368.54775807"sv); //LUNA max position
}

#endif //AWL_DECIMAL_128
