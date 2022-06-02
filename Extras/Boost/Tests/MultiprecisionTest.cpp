/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <iomanip>
#include <limits>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include "Awl/String.h"
#include "BoostExtras/DecimalData.h"

#include "Awl/Testing/UnitTest.h"

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
    AWT_FLAG(test_square);
    AWT_ATTRIBUTE(size_t, square_count, 5);

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

    if (test_square)
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
}

AWT_EXAMPLE(MultiprecisionDecimalData)
{
    awl::BoostDecimalData<bmp::uint128_t, 4> data;
}
