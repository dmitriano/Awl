#pragma once

#include "Awl/Decimal.h"

#if (defined(__GNUC__) || defined(__clang__)) && defined(__SIZEOF_INT128__)
    #define AWL_INT_128 1
#endif

#if !defined(AWL_INT_128) && defined(AWL_BOOST)
    #include "BoostExtras/MultiprecisionDecimalData.h"
    #include "BoostExtras/Io/MultiprecisionTypeName.h"
#endif

namespace awl
{
    //using decimal = awl::decimal<uint64_t, 4>;

#if defined(AWL_INT_128)

    template <uint8_t exp_len>
    using decimal128 = awl::decimal<__uint128_t, exp_len>;

    #define AWL_DECIMAL_128 1

#elif defined(AWL_BOOST)

    template <uint8_t exp_len>
    using decimal128 = awl::decimal<boost::multiprecision::uint128_t, exp_len, awl::MultiprecisionDecimalData>;

    #define AWL_DECIMAL_128 1

#else

    // 128 bit integer is not accessible.
    // Leave AWL_DECIMAL_128 undefined.

#endif
}
