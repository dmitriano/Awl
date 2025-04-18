/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Helpers/RwTest.h"

#include "Awl/Decimal.h"
#include "Awl/Testing/UnitTest.h"

#ifdef AWL_BOOST
#include "BoostExtras/MultiprecisionDecimalData.h"
#endif

#include <functional>

#ifdef AWL_INT_128

#define GCC_SECTION(test_name) \
    { Test<awl::decimal<__uint128_t, 4>> test(context); test.test_name(); } \
    { Test<awl::decimal<__uint128_t, 5>> test(context); test.test_name(); } \
    { Test<awl::decimal<__uint128_t, 6>> test(context); test.test_name(); }

#else

#define GCC_SECTION(test_name)

#endif

#ifdef AWL_BOOST

namespace bmp = boost::multiprecision; 

#define BOOST_SECTION(test_name) \
    { Test<awl::decimal<bmp::uint128_t, 4, awl::MultiprecisionDecimalData>> test(context); test.test_name(); } \
    { Test<awl::decimal<bmp::uint128_t, 5, awl::MultiprecisionDecimalData>> test(context); test.test_name(); } \
    { Test<awl::decimal<bmp::uint128_t, 6, awl::MultiprecisionDecimalData>> test(context); test.test_name(); }

#else

#define BOOST_SECTION(test_name)

#endif

#if true

#define BUILTIN_SECTION(test_name) \
    { Test<awl::decimal<uint64_t, 4>> test(context); test.test_name(); } \
    { Test<awl::decimal<uint64_t, 5>> test(context); test.test_name(); } \
    { Test<awl::decimal<uint64_t, 6>> test(context); test.test_name(); }

#else

#define BUILTIN_SECTION(test_name)

#endif

#define LOCAL_TEST(test_name) AWL_TEST(test_name) \
{ \
    BUILTIN_SECTION(test_name) \
    GCC_SECTION(test_name) \
    BOOST_SECTION(test_name) \
}

using namespace std::literals;

using namespace awl::testing;

namespace
{
    using Decimal64 = awl::decimal<uint64_t, 4>;

#ifdef AWL_INT_128

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
    private:

        const awl::testing::TestContext& context;

    public:

        Test(const awl::testing::TestContext& ctx) : context(ctx) {}

        template <class C>
        void TestStringConversion(std::basic_string_view<C> sample, std::basic_string_view<C> result = {})
        {
            Decimal d(sample);

            std::basic_ostringstream<C> out;

            out << d;

            auto fixed_string = out.str();

            if (result.empty())
            {
                AWL_ASSERT(fixed_string == sample);
            }
            else
            {
                AWL_ASSERT(fixed_string == result);
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
                AWL_FAILM("Decimal did not throw.");
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

            AWL_ASSERT(fixed_string == _T("10.12800"));
        }

        void DecimalRescale()
        {
            AWL_ASSERT(Decimal::zero().rescale(3).to_astring() == "0.000");

            Decimal d("123.45678"sv);

            d = d.rescale(7);

            AWL_ASSERT(d.to_astring() == "123.4567800");

            d = d.rescale(5);

            AWL_ASSERT(d.to_astring() == "123.45678");

            d *= Decimal(100000, 0);

            d = d.rescale(0);

            AWL_ASSERT(d.to_astring() == "12345678");

            //9 + 8 = 17
            d = d.rescale(9);

            AWL_ASSERT(d.to_astring() == "12345678.000000000");

            d = d.rescale(0);

            AWL_ASSERT(d.to_astring() == "12345678");

            //TO DO: Why 10 works?
            d = d.rescale(10);

            AWL_ASSERT(d.to_astring() == "12345678.0000000000");
        }

        void DecimalTrim()
        {
            Decimal d("123.45678"sv);

            d = d.truncate(3);

            AWL_ASSERT(d.to_astring() == "123.456");

            d = d.truncate(0);

            AWL_ASSERT(d.to_astring() == "123");
        }

        void DecimalCeil()
        {
            {
                Decimal d("123.45678"sv);

                d = d.ceil(3);

                AWL_ASSERT(d.to_astring() == "123.457");

                d = d.ceil(0);

                AWL_ASSERT(d.to_astring() == "124");
            }

            {
                Decimal d("123.00000"sv);

                //it is constructed as normalized.
                AWL_ASSERT(d.to_astring() == "123");

                d = d.extend(5);

                AWL_ASSERT(d.to_astring() == "123.00000");

                d = d.ceil(3);

                AWL_ASSERT(d.to_astring() == "123.000");

                d = d.ceil(0);

                AWL_ASSERT(d.to_astring() == "123");
            }
        }

        void DecimalCompare()
        {
            AWL_ASSERT(Decimal("123.456789"sv) == Decimal("123.456789000"sv));
            AWL_ASSERT(Decimal("0"sv) == Decimal("0"sv));
            AWL_ASSERT(Decimal("123"sv) == Decimal("123.000"sv));
            AWL_ASSERT(Decimal("123"sv) <= Decimal("123.000"sv));
            AWL_ASSERT(Decimal("123"sv) >= Decimal("123.000"sv));

            AWL_ASSERT(Decimal("1"sv) != Decimal("2.000"sv));

            AWL_ASSERT(Decimal("0.5"sv) < Decimal("2"sv));
            AWL_ASSERT(Decimal("0.596"sv) < Decimal("2.232"sv));
            AWL_ASSERT(!(Decimal("0.596"sv) >= Decimal("2.232"sv)));
            AWL_ASSERT(Decimal("1"sv) < Decimal("2.000"sv));
            AWL_ASSERT(Decimal("2.000"sv) > Decimal("1.000"sv));
            AWL_ASSERT(Decimal("1"sv) <= Decimal("2.000"sv));
            AWL_ASSERT(Decimal("2.000"sv) >= Decimal("1.000"sv));

            AWL_ASSERT(Decimal("1"sv) < 2.000);
            AWL_ASSERT(Decimal("2.000"sv) > 1.000);
            AWL_ASSERT(Decimal("1"sv) <= 2.000);
            AWL_ASSERT(Decimal("2.000"sv) >= 1.000);

            //From BZRX/USDT order data
            AWL_ASSERT(Decimal("0.2678"sv) < Decimal("0.3000"sv));
            AWL_ASSERT(Decimal("0.2678"sv) < Decimal("0.3400"sv));
            AWL_ASSERT(Decimal("0.3000"sv) < Decimal("0.3143"sv));
            AWL_ASSERT(Decimal("0.3143"sv) < Decimal("0.3400"sv));
        }

        void DecimalCast()
        {
            AWL_ASSERT(Decimal("1"sv).template cast<double>() == 1.0);
            AWL_ASSERT(Decimal("1"sv).template cast<float>() == 1.0f);
            AWL_ASSERT(Decimal("1"sv).template cast<int>() == 1);

            AWL_ASSERT(Decimal("1"sv) == 1.0);
        }

        void DecimalArithmeticOperators()
        {
            //Addition and subtraction
            AWL_ASSERT(Decimal("1.0"sv) + Decimal("2.000"sv) == Decimal("3"sv));
            AWL_ASSERT(Decimal("1.05"sv) + Decimal("2.005"sv) == Decimal("3.055"sv));
            AWL_ASSERT(Decimal("1.05"sv) - Decimal("0.05"sv) == Decimal("1"sv));
            AWL_ASSERT((Decimal("1.05"sv) += Decimal("2.005"sv)) == Decimal("3.055"sv));
            AWL_ASSERT((Decimal("1.05"sv) -= Decimal("0.05"sv)) == Decimal("1"sv));
            AWL_ASSERT((Decimal("1"sv) += 2.00) == Decimal("3.0"sv));
            AWL_ASSERT((Decimal("5"sv) -= 2.00) == Decimal("3"sv));

            {
                Decimal d("1.0"sv);

                d += Decimal("2.0"sv);

                AWL_ASSERT(d == Decimal("3.0"sv));

                d -= Decimal("2.0"sv);

                AWL_ASSERT(d == Decimal("1.0"sv));
            }

            //Multiplication and Division
            AWL_ASSERT(Decimal::make_decimal(Decimal("2.0"sv) * Decimal("3.000"sv), 5) == Decimal("6"sv));
            AWL_ASSERT(Decimal::make_decimal(Decimal("6.0"sv) / Decimal("2.000"sv), 5) == Decimal("3"sv));
            AWL_ASSERT((Decimal("2.0"sv) *= Decimal("3.000"sv)) == Decimal("6"sv));
            AWL_ASSERT((Decimal("6.0"sv) /= Decimal("2.000"sv)) == Decimal("3"sv));
            AWL_ASSERT((Decimal("2.0"sv) *= 3.000) == Decimal("6"sv));
            AWL_ASSERT((Decimal("6.0"sv) /= 2.000) == Decimal("3"sv));

            AWL_ASSERT(awl::multiply(Decimal("7"), Decimal("33")) == Decimal("231"));
            //With double it is probably something like 0.23099999999999998
            AWL_ASSERT(awl::multiply(Decimal("0.7"), Decimal("0.33")) == Decimal("0.231"));
        }

        void DecimalRound()
        {
            {
                const Decimal d33 = Decimal::make_decimal(1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("0.33"));

                const Decimal d66 = Decimal::make_decimal(2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("0.66"));

                const Decimal d67 = Decimal::make_rounded(2.0 / 3.0, 2);
                AWL_ASSERT(d67 == Decimal("0.67"));

                const Decimal d3 = Decimal::make_rounded(3, 1);
                AWL_ASSERT(d3 == Decimal("3"));

                const Decimal d3d = Decimal::make_rounded(3.0, 1);
                AWL_ASSERT(d3d == Decimal("3"));
            }

            {
                const Decimal d33 = Decimal::make_decimal(-1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("-0.33"));

                const Decimal d66 = Decimal::make_decimal(-2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("-0.66"));

                const Decimal d67 = Decimal::make_rounded(-2.0 / 3.0, 2);
                AWL_ASSERT(d67 == Decimal("-0.67"));

                const Decimal d3 = Decimal::make_rounded(-3, 1);
                AWL_ASSERT(d3 == Decimal("-3"));

                const Decimal d3d = Decimal::make_rounded(-3.0, 1);
                AWL_ASSERT(d3d == Decimal("-3"));
            }

            {
                const Decimal d33 = Decimal::make_ceiled(1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("0.34"));

                const Decimal d66 = Decimal::make_ceiled(2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("0.67"));

                const Decimal d3 = Decimal::make_ceiled(3, 1);
                AWL_ASSERT(d3 == Decimal("3"));

                const Decimal d3d = Decimal::make_ceiled(3.0, 1);
                AWL_ASSERT(d3d == Decimal("3"));
            }

            {
                const Decimal d33 = Decimal::make_ceiled(-1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("-0.33"));

                const Decimal d66 = Decimal::make_ceiled(-2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("-0.66"));

                const Decimal d3 = Decimal::make_ceiled(-3, 1);
                AWL_ASSERT(d3 == Decimal("-3"));

                const Decimal d3d = Decimal::make_ceiled(-3.0, 1);
                AWL_ASSERT(d3d == Decimal("-3"));
            }

            {
                const Decimal d33 = Decimal::make_floored(1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("0.33"));

                const Decimal d66 = Decimal::make_floored(2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("0.66"));

                const Decimal d3 = Decimal::make_floored(3, 1);
                AWL_ASSERT(d3 == Decimal("3"));

                const Decimal d3d = Decimal::make_floored(3.0, 1);
                AWL_ASSERT(d3d == Decimal("3"));
            }

            {
                const Decimal d33 = Decimal::make_floored(-1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("-0.34"));

                const Decimal d66 = Decimal::make_floored(-2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("-0.67"));

                const Decimal d3 = Decimal::make_floored(-3, 1);
                AWL_ASSERT(d3 == Decimal("-3"));

                const Decimal d3d = Decimal::make_floored(-3.0, 1);
                AWL_ASSERT(d3d == Decimal("-3"));
            }

            {
                const Decimal d33 = Decimal::make_truncated(1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("0.33"));

                const Decimal d66 = Decimal::make_truncated(2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("0.66"));

                const Decimal d3 = Decimal::make_truncated(3, 1);
                AWL_ASSERT(d3 == Decimal("3"));
            }

            {
                const Decimal d33 = Decimal::make_truncated(-1.0 / 3.0, 2);
                AWL_ASSERT(d33 == Decimal("-0.33"));

                const Decimal d66 = Decimal::make_truncated(-2.0 / 3.0, 2);
                AWL_ASSERT(d66 == Decimal("-0.66"));

                const Decimal d3 = Decimal::make_truncated(-3, 1);
                AWL_ASSERT(d3 == Decimal("-3"));
            }

            {
                Decimal d(2);
                d = -1.0 / 3.0;
                AWL_ASSERT(d == Decimal("-0.33"));

                d = -2.0 / 3.0;
                AWL_ASSERT(d == Decimal("-0.66"));

                d = -3.0;
                AWL_ASSERT(d == Decimal(_T("-3")));
            }

            {
                Decimal d;
                d = -1.0 / 3.0;
                AWL_ASSERT(d == Decimal("0"));

                d = -2.0 / 3.0;
                AWL_ASSERT(d == Decimal("0"));

                d = -3.0;
                AWL_ASSERT(d == Decimal(_T("-3")));
            }

            AWL_ASSERT(Decimal{} == Decimal::zero());
        }

        void DecimalReadWrite()
        {
            helpers::TestReadWrite<Decimal>(context, Decimal("0"));
            helpers::TestReadWrite<Decimal>(context, Decimal("0.3"));
            helpers::TestReadWrite<Decimal>(context, Decimal("3.3"));
            helpers::TestReadWrite<Decimal>(context, Decimal("33.33"));
            helpers::TestReadWrite<Decimal>(context, Decimal("333.333"));
            helpers::TestReadWrite<Decimal>(context, std::numeric_limits<Decimal>::max());
            helpers::TestReadWrite<Decimal>(context, std::numeric_limits<Decimal>::min());
        }
    };
}

LOCAL_TEST(DecimalStringConversion)

LOCAL_TEST(DecimalRescale)

LOCAL_TEST(DecimalCompare)

LOCAL_TEST(DecimalCast)

LOCAL_TEST(DecimalArithmeticOperators)

LOCAL_TEST(DecimalTrim)

LOCAL_TEST(DecimalCeil)

LOCAL_TEST(DecimalRound)

LOCAL_TEST(DecimalReadWrite)

AWL_TEST(DecimalRescaleOverflow)
{
    Test<Decimal64> test(context);

    Decimal64 d("12345678.0000000000"sv);

    test.CheckTrows([&]()
    {
        //TO DO: Why 11?
        d.rescale(11);
    });
}

AWL_TEST(DecimalStringConversionOverflow)
{
    Test<Decimal64> test(context);

    test.CheckTrows([&]()
    {
        test.TestStringConversion("1234.123456789123456"sv);
    });
}

AWL_TEST(DecimalLimits)
{
    Test<Decimal64> test(context);

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

AWL_TEST(DecimalMinMax)
{
    AWL_UNUSED_CONTEXT;

    Decimal64 max = std::numeric_limits<Decimal64>::max();
    Decimal64 min = std::numeric_limits<Decimal64>::min();

    AWL_ASSERT(min < max);
    AWL_ASSERT(max > min);

    AWL_ASSERT(max == max);
    AWL_ASSERT(max - Decimal64(1, 0) < max);
    AWL_ASSERT(Decimal64(1, 18) < max);
    AWL_ASSERT(Decimal64(-1, 18) < max);
    AWL_ASSERT(Decimal64::zero() < max);
    AWL_ASSERT(Decimal64::make_decimal(1, Decimal64::max_exponent()) < max);

    AWL_ASSERT(min == min);
    AWL_ASSERT(min < min + Decimal64(1, 0));
    AWL_ASSERT(min < Decimal64::zero());

    AWL_ASSERT(min <= max);
    AWL_ASSERT(max >= min);

    AWL_ASSERT(max != max - Decimal64(1, 0));
    AWL_ASSERT(max - Decimal64(1, 0) <= max);
    AWL_ASSERT(Decimal64::zero() <= max);
    AWL_ASSERT(Decimal64::make_decimal(1, Decimal64::max_exponent()) <= max);

    AWL_ASSERT(min != min + Decimal64(1, 0));
    AWL_ASSERT(min <= min + Decimal64(1, 0));
    AWL_ASSERT(min <= Decimal64::zero());

    for (uint8_t precision = 0; precision <= Decimal64::max_exponent(); ++precision)
    {
        AWL_ASSERT(Decimal64(1, precision) > min);
        AWL_ASSERT(Decimal64(-1, precision) > min);
        AWL_ASSERT(Decimal64(1, precision) >= min);
        AWL_ASSERT(Decimal64(-1, precision) >= min);

        AWL_ASSERT(Decimal64(1, precision) < max);
        AWL_ASSERT(Decimal64(-1, precision) < max);
        AWL_ASSERT(Decimal64(1, precision) <= max);
        AWL_ASSERT(Decimal64(-1, precision) <= max);

        AWL_ASSERT(Decimal64(1, precision) > Decimal64::zero());
        AWL_ASSERT(Decimal64(-1, precision) < Decimal64::zero());
        AWL_ASSERT(Decimal64(-1, precision) < Decimal64(1, precision));
        AWL_ASSERT(Decimal64(1, precision) > Decimal64(-1, precision));

        AWL_ASSERT(Decimal64(1, precision) >= Decimal64::zero());
        AWL_ASSERT(Decimal64(-1, precision) <= Decimal64::zero());
        AWL_ASSERT(Decimal64(-1, precision) <= Decimal64(1, precision));
        AWL_ASSERT(Decimal64(1, precision) >= Decimal64(-1, precision));
    }
}

//It is not enough to store some values from Binance.
AWL_EXAMPLE(DecimalBinanceValues)
{
    Test<Decimal64> test(context);
    
    constexpr uint64_t max_uint = std::numeric_limits<uint64_t>::max();

    context.out << _T("uint64_t max: ") << max_uint << ", decimal digits: " << static_cast<size_t>(std::log10(max_uint)) << std::endl;

    context.out << _T("max decimal man: ") << Decimal64::max_mantissa() << _T(", max decimal exp: ") << Decimal64::max_exponent() << std::endl;

    test.CheckConstructorTrows("8426879770.74001051"sv); //BTC day volume

    test.CheckConstructorTrows("92233720368.54775807"sv); //LUNA max position
}

namespace
{
    Decimal64 square(Decimal64 val)
    {
        return awl::multiply(val, val);
    }
}

AWL_EXAMPLE(DecimalSimpleCalculation)
{
    const Decimal64 src_side("1.525");

    const Decimal64 src_area = square(src_side);

    context.out << _T("Src. side: ") << src_side << _T(", src. area: ") << src_area << std::endl;

    {
        const Decimal64 a("0.85");
        const Decimal64 b("0.67");
        const Decimal64 c("0.80");

        const Decimal64 area = awl::multiply((awl::multiply(a, b) + awl::multiply(b, c) + awl::multiply(a, c)), Decimal64("2"));

        context.out << "area #1: " << area << std::endl;
    }

    {
        const Decimal64 a("0.70");
        const Decimal64 b("0.60");
        const Decimal64 c("0.20");

        const Decimal64 area = awl::multiply(a, b) + awl::multiply((awl::multiply(b, c) + awl::multiply(a, c)), Decimal64("2"));

        context.out << "area #2: " << area << std::endl;
    }
}
