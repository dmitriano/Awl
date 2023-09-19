/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Decimal.h"

#include <chrono>

namespace awl
{
    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate = BuiltinDecimalData>
    constexpr std::chrono::nanoseconds decimal_to_nanoseconds(awl::decimal<UInt, exp_len, DataTemplate> dec)
    {
        //using Decimal = awl::decimal<UInt, exp_len, DataTemplate>;
        //const Decimal num_dec = awl::multiply(dec, Decimal(Duration::rep::den, 0));
        //typename Duration::rep rep = dum_dec.mantissa() / Duration::rep::num;
        //return Duration(static_cast<typename Duration::rep>(dec.extend(9).mantissa()));

        return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(dec.extend(9).mantissa()));
    }
}
