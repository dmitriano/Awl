/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "Tests/Helpers/RwTest.h"

#include "Awl/String.h"
#include "Awl/StringFormat.h"
#include "Awl/Separator.h"
#include "BoostExtras/MultiprecisionDecimalData.h"
#include "BoostExtras/MultiprecisionTraits.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/small_vector.hpp>

#include <iostream>
#include <iomanip>
#include <limits>

using namespace awl::testing;
namespace bmp = boost::multiprecision;

static_assert(awl::helpers::DecimalConstants<bmp::uint128_t, 4, 16>::man_len == 59u + 64u);

AWL_EXAMPLE(MultiprecisionSize)
{
    auto max = std::numeric_limits<bmp::uint128_t>::max();

    size_t size = sizeof(max);
    
    context.out <<
        _T("max: ") << max << std::endl <<
        _T("size: ") << size << std::endl;
}

AWL_EXAMPLE(MultiprecisionDecimalConstants)
{
    using Constants = awl::helpers::DecimalConstants<bmp::uint128_t, 4, 16>;

    context.out << "man_len: " << Constants::man_len;
}

AWL_EXAMPLE(MultiprecisionDecFloat)
{
    AWL_ATTRIBUTE(std::string, number, "92233720368.54775807"); //LUNA max position
    AWL_ATTRIBUTE(size_t, iter_count, 1000000u);
    AWL_ATTRIBUTE(size_t, div_count, 50);
    AWL_FLAG(no_square);
    AWL_ATTRIBUTE(size_t, square_count, 5);
    AWL_FLAG(no_div);

    try
    {
        //IEEE 754 double stores 2^53 without losing precision that is 15 decimal digits,
        //Simple examples of numbers which cannot be exactly represented in binary floating - point numbers include 1 / 3, 2 / 3, 1 / 5
        constexpr unsigned lenght = 30;

        using Decimal = bmp::number<bmp::cpp_dec_float<lenght, int16_t>>;

        const Decimal a(number);
        const double b = std::stod(number);

        context.logger.debug(awl::format() <<
            "sizeof(Decimal): " << sizeof(Decimal) << awl::format::endl <<
            "boost:\t" << std::fixed << a << awl::format::endl <<
            "double:\t" << std::fixed << b << awl::format::endl <<
            "cast:\t" << std::fixed << static_cast<double>(a));

        Decimal a_sum = 0;
        double b_sum = 0;

        for (size_t i = 0; i < iter_count; ++i)
        {
            a_sum += a;

            b_sum += b;
        }

        const Decimal a_product = a * iter_count;
        const double b_product = b * iter_count;

        context.logger.debug(awl::format() <<
            "decimal sum: " << a_sum << awl::format::endl <<
            "decimal product: " << a_product << awl::format::endl <<
            "double sum: " << b_sum << awl::format::endl <<
            "double product: " << b_product);

        AWL_ASSERT(a_sum == a_product);

        if (!no_div)
        {
            Decimal a_quotient = a;

            context.logger.debug(awl::format() << "decimal quotient: ");

            //while (a_quotient != 0)
            for (size_t i = 0; i < div_count; ++i)
            {
                a_quotient /= 10;

                context.logger.debug(awl::format() << a_quotient);
            }
        }

        if (!no_square)
        {
            Decimal a_square = a;
            Decimal a_sqrt = a;

            //std::cout << std::fixed << std::setprecision(lenght * square_count / 2);

            for (size_t i = 0; i < square_count; ++i)
            {
                a_square = a_square * a_square;

                a_sqrt = bmp::sqrt(a_sqrt);

                Decimal sum = a_square + a_sqrt;

                context.logger.debug(awl::format() <<
                    "square: " << a_square << awl::format::endl <<
                    "sqrt: " << a_sqrt << awl::format::endl <<
                    "square + sqrt: " << sum);
            }
        }
    }
    catch (const std::exception& e)
    {
        context.logger.debug(awl::format() << "Exception: " << e.what());

        AWL_FAIL;
    }
}

//AWL_EXAMPLE(MultiprecisionDecBackend)
//{
//    AWL_ATTRIBUTE(std::string, number, "92233720368.54775807"); //LUNA max position
//
//    constexpr unsigned lenght = 30;
//
//    using Decimal = bmp::number<bmp::cpp_dec_float<lenght, int16_t>>;
//
//    Decimal d(number);
//
//    //bmp::cpp_int n;
//
//    std::cout << "exponent: " << d.backend().exponent();
//}

namespace
{
    using Data = awl::MultiprecisionDecimalData<bmp::uint128_t, 4>;

    static_assert(Data().positive());
    static_assert(!Data(false, 3, 15).positive());

    static_assert(Data(true, 3, 15).man() == 15);
    static_assert(Data(true, 3, 1).man() == 1);
    static_assert(Data(true, 3, 0).man() == 0);

    static_assert(Data(true, 3, 15).exp() == 3);
    static_assert(Data(true, 1, 15).exp() == 1);
    static_assert(Data(true, 0, 15).exp() == 0);
}

AWL_EXAMPLE(MultiprecisionDecimalData)
{
    using UInt = bmp::uint128_t;
    
    awl::MultiprecisionDecimalData<UInt, 4> d(true, 2, 105);

    context.out << d.man() << ", " << d.exp() << std::endl;

    d.set_man(UInt(105) * UInt(10));
    
    d.set_exp(3);

    context.out << d.man() << ", " << d.exp() << std::endl;
}

namespace
{
    void PrintVector(awl::ostream& out, const std::vector<uint8_t>& v)
    {
        out << "Vector size: " << v.size() << std::endl;

        out << "Vector elements:" << std::endl;

        awl::separator sep(_T(','));

        for (unsigned char val : v)
        {
            out << sep << "0x" << std::hex << std::setfill(_T('0')) << std::setw(2) << val;
        }

        out << std::endl;
    }
}

namespace
{
    template <class Int>
    void ParseNumber(const std::string& number, Int& i)
    {
        if (number == "max")
        {
            i = std::numeric_limits<Int>::max();
        }
        else if (number == "min")
        {
            i = std::numeric_limits<Int>::min();
        }
        else
        {
            i = Int(number);
        }
    }

    template <class Int>
    void TestMultiprecisionIntImportExportBits(const awl::testing::TestContext& context)
    {
        AWL_ATTRIBUTE(std::string, number, "max");
        AWL_FLAG(negate);

        Int i;

        ParseNumber(number, i);

        if (negate)
        {
            i.backend().negate();
        }

        context.out <<
            _T("number: ") << i << std::endl <<
            _T("sign:") << i.sign() << std::endl;

        // export into 8-bit unsigned values, most significant bit first:
        std::vector<uint8_t> v;
        export_bits(i, std::back_inserter(v), 8);

        PrintVector(context.out, v);

        // import back again, and check for equality:
        Int j;
        import_bits(j, v.begin(), v.end());

        if (i < 0)
        {
            j.backend().negate();
        }

        AWL_ASSERT(i == j);
    }
}

AWL_TEST(MultiprecisionImportExportBitsInt128)
{
    TestMultiprecisionIntImportExportBits<bmp::int128_t>(context);
}

AWL_TEST(MultiprecisionImportExportBitsUInt128)
{
    TestMultiprecisionIntImportExportBits<bmp::uint128_t>(context);
}

AWL_TEST(MultiprecisionImportExportBitsInt256)
{
    TestMultiprecisionIntImportExportBits<bmp::int256_t>(context);
}

AWL_TEST(MultiprecisionImportExportBitsUInt256)
{
    TestMultiprecisionIntImportExportBits<bmp::uint256_t>(context);
}

AWL_TEST(MultiprecisionImportExportBitsInt512)
{
    TestMultiprecisionIntImportExportBits<bmp::int512_t>(context);
}

AWL_TEST(MultiprecisionImportExportBitsUInt512)
{
    TestMultiprecisionIntImportExportBits<bmp::uint512_t>(context);
}

AWL_TEST(MultiprecisionImportExportBitsInt1024)
{
    TestMultiprecisionIntImportExportBits<bmp::int1024_t>(context);
}

AWL_TEST(MultiprecisionImportExportBitsUInt1024)
{
    TestMultiprecisionIntImportExportBits<bmp::uint1024_t>(context);
}

AWL_TEST(MultiprecisionFloatImportExportBits)
{
    //using Decimal = bmp::number<bmp::cpp_dec_float<30, int16_t>>;
    //Decimal i("33.3");
    using boost::multiprecision::cpp_bin_float_100;
    using boost::multiprecision::cpp_int;

    // Create a cpp_bin_float to import/export:
    cpp_bin_float_100 f(1);
    f /= 3;
    // export into 8-bit unsigned values, most significant bit first:
    std::vector<unsigned char> v;
    export_bits(cpp_int(f.backend().bits()), std::back_inserter(v), 8);

    PrintVector(context.out, v);

    // Grab the exponent as well:
    int e = f.backend().exponent();
    // Import back again, and check for equality, we have to procede via
    // an intermediate integer:
    cpp_int i;
    import_bits(i, v.begin(), v.end());
    cpp_bin_float_100 g(i);
    g.backend().exponent() = e;
    
    AWL_ASSERT(f == g);
}

namespace
{
    template <class T>
    void TestInt(const TestContext& context)
    {
        //Does not work.
        //if constexpr (boost::is_signed<T>::value)

        helpers::TestReadWrite(context, T(-1));
        helpers::TestReadWrite(context, T(-3));
        helpers::TestReadWrite(context, T(-33));
        helpers::TestReadWrite(context, T(-333));
        helpers::TestReadWrite(context, T(-3333));

        helpers::TestReadWrite(context, std::numeric_limits<T>::max());
        helpers::TestReadWrite(context, std::numeric_limits<T>::min());
        helpers::TestReadWrite(context, T(0));
        helpers::TestReadWrite(context, T(1));
        helpers::TestReadWrite(context, T(3));
        helpers::TestReadWrite(context, T(33));
        helpers::TestReadWrite(context, T(333));
        helpers::TestReadWrite(context, T(3333));
    }
}

AWL_TEST(MultiprecisionReadWrite)
{
    TestInt<bmp::uint128_t>(context);
    TestInt<bmp::uint256_t>(context);
    TestInt<bmp::uint512_t>(context);
    //TestInt<bmp::uint1024_t>(context);

    TestInt<bmp::int128_t>(context);
    TestInt<bmp::int256_t>(context);
    TestInt<bmp::int512_t>(context);
    //TestInt<bmp::int1024_t>(context);
}

AWL_TEST(MultiprecisionContainer)
{
    AWL_ATTRIBUTE(std::string, number, "max");

    using Int = bmp::uint128_t;

    Int i;

    ParseNumber(number, i);

    {
        constexpr std::size_t size = 5;

        boost::container::static_vector<std::uint8_t, size> v;

        try
        {
            export_bits(i, std::back_inserter(v), 8);

            AWL_FAILM("static_vector did not throw.");
        }
        catch (boost::container::bad_alloc&)
        {
        }
    }

    {
        constexpr std::size_t size = awl::helpers::multiprecision_descriptor<Int>::size;

        //boost::container::static_vector<std::uint8_t, size> v;
        boost::container::small_vector<std::uint8_t, size> v;

        export_bits(i, std::back_inserter(v), 8);
 
        // import back again, and check for equality:
        Int j;
        import_bits(j, v.begin(), v.end());
    
        AWL_ASSERT(i == j);
    }
}