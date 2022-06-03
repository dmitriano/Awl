/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "Tests/Helpers/RwTest.h"

#include "Awl/String.h"
#include "Awl/Separator.h"
#include "BoostExtras/DecimalData.h"

#include "Awl/Testing/UnitTest.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

#include <iostream>
#include <iomanip>
#include <limits>

using namespace awl::testing;
namespace bmp = boost::multiprecision;

static_assert(awl::helpers::DecimalConstants<bmp::uint128_t, 4, 16>::man_len == 59u + 64u);

AWT_EXAMPLE(MultiprecisionSize)
{
    auto max = std::numeric_limits<bmp::uint128_t>::max();

    size_t size = sizeof(max);
    
    context.out <<
        _T("max: ") << max << std::endl <<
        _T("size: ") << size << std::endl;
}

AWT_EXAMPLE(MultiprecisionDecimalConstants)
{
    using Constants = awl::helpers::DecimalConstants<bmp::uint128_t, 4, 16>;

    context.out << "man_len: " << Constants::man_len;
}

AWT_EXAMPLE(MultiprecisionDecFloat)
{
    AWT_ATTRIBUTE(std::string, number, "92233720368.54775807"); //LUNA max position
    AWT_ATTRIBUTE(size_t, iter_count, 1000000u);
    AWT_ATTRIBUTE(size_t, div_count, 50);
    AWT_FLAG(no_square);
    AWT_ATTRIBUTE(size_t, square_count, 5);
    AWT_FLAG(no_div);

    //IEEE 754 double stores 2^53 without losing precision that is 15 decimal digits,
    //Simple examples of numbers which cannot be exactly represented in binary floating - point numbers include 1 / 3, 2 / 3, 1 / 5
    constexpr unsigned lenght = 30;
    
    using Decimal = bmp::number<bmp::cpp_dec_float<lenght, int16_t>>;

    const Decimal a(number);
    const double b = std::stod(number);

    std::cout <<
        "sizeof(Decimal): " << sizeof(Decimal) << std::endl <<
        "boost:\t" << std::fixed << a << std::endl <<
        "double:\t" << std::fixed << b << std::endl <<
        "cast:\t" << std::fixed << static_cast<double>(a) <<
        std::endl;

    //context.out <<
    //    _T("sizeof(Decimal): ") << sizeof(Decimal) << std::endl <<
    //    _T("boost: ") << std::fixed << std::setprecision(lenght) << decimal << std::endl <<
    //    _T("double: ") << std::fixed << std::setprecision(lenght) << dbl <<
    //    std::endl;

    Decimal a_sum = 0;
    double b_sum = 0;

    for (size_t i = 0; i < iter_count; ++i)
    {
        a_sum += a;
        
        b_sum += b;
    }

    const Decimal a_product = a * iter_count;
    const double b_product = b * iter_count;

    std::cout <<
        "decimal sum: " << a_sum << std::endl <<
        "decimal product: " << a_product << std::endl <<
        "double sum: " << b_sum << std::endl <<
        "double product: " << b_product << std::endl;

    AWT_ASSERT(a_sum == a_product);

    if (!no_div)
    {
        Decimal a_quotient = a;

        std::cout << "decimal quotient: " << std::endl;

        //while (a_quotient != 0)
        for (size_t i = 0; i < div_count; ++i)
        {
            a_quotient /= 10;

            std::cout << a_quotient << std::endl;
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

            std::cout <<
                "square: " << a_square << std::endl <<
                "sqrt: " << a_sqrt << std::endl <<
                "square + sqrt: " << a_square + a_sqrt << std::endl;
        }
    }
}

//AWT_EXAMPLE(MultiprecisionDecBackend)
//{
//    AWT_ATTRIBUTE(std::string, number, "92233720368.54775807"); //LUNA max position
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
    using Data = awl::BoostDecimalData<bmp::uint128_t, 4>;

    static_assert(Data().positive());
    static_assert(!Data(false, 3, 15).positive());

    static_assert(Data(true, 3, 15).man() == 15);
    static_assert(Data(true, 3, 1).man() == 1);
    static_assert(Data(true, 3, 0).man() == 0);

    static_assert(Data(true, 3, 15).exp() == 3);
    static_assert(Data(true, 1, 15).exp() == 1);
    static_assert(Data(true, 0, 15).exp() == 0);
}

AWT_EXAMPLE(MultiprecisionDecimalData)
{
    using UInt = bmp::uint128_t;
    
    awl::BoostDecimalData<UInt, 4> d(true, 2, 105);

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

AWT_EXAMPLE(MultiprecisionIntImportExportBits)
{
    AWT_ATTRIBUTE(int, number, std::numeric_limits<int>::max());
    AWT_FLAG(max);

    using Int = bmp::uint128_t;
    //using Int = bmp::int128_t;

    // Create a cpp_int with just a couple of bits set:

    Int i = max ? std::numeric_limits<Int>::max() : Int(number);

    i.backend().negate();

    context.out << _T("sign:") << i.sign() << std::endl;

    // export into 8-bit unsigned values, most significant bit first:
    std::vector<uint8_t> v;
    export_bits(i, std::back_inserter(v), 8);

    PrintVector(context.out, v);

    // import back again, and check for equality:
    Int j;
    import_bits(j, v.begin(), v.end());
    
    AWT_ASSERT(i == j);
}

AWT_EXAMPLE(MultiprecisionFloatImportExportBits)
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
    
    AWT_ASSERT(f == g);
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

AWT_TEST(MultiprecisionReadWrite)
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
