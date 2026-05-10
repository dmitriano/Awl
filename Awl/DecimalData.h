/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DecimalConstants.h"

#include <cstdint>
#include <cassert>
#include <limits>
namespace awl
{
    template <typename UInt, uint8_t exp_len>
    class BuiltinDecimalData
    {
    private:

        using Constants = helpers::DecimalConstants<UInt, exp_len>;

        struct Pack
        {
            //0 - positive, 1 - negative
            UInt sign : Constants::sign_len;
            UInt exp : Constants::exp_len;
            UInt man : Constants::man_len;
        };

        static_assert(sizeof(Pack) == sizeof(UInt));

    public:

        using Rep = UInt;

        using Int = std::make_signed_t<UInt>;

    private:

        union Data
        {
            constexpr Data(bool sign, uint8_t exp, UInt man) : pack{ sign ? 0u : 1u, exp, man } {}

            Pack pack;
            Rep rep;
        };

    public:

        constexpr BuiltinDecimalData() : BuiltinDecimalData(true, 0, 0) {}
        
        constexpr BuiltinDecimalData(bool sign, uint8_t exp, UInt man) : _data(sign, exp, man) {}

        constexpr bool positive() const
        {
            return _data.pack.sign == 0;
        }

        constexpr void set_positive(bool val)
        {
            _data.pack.sign = val ? 0u : 1u;
        }

        constexpr uint8_t exp() const
        {
            return _data.pack.exp;
        }

        constexpr void set_exp(UInt val)
        {
            _data.pack.exp = val;
        }

        constexpr UInt man() const
        {
            return _data.pack.man;
        }

        constexpr void set_man(UInt val)
        {
            _data.pack.man = val;
        }

        static constexpr BuiltinDecimalData from_bits(Rep val)
        {
            BuiltinDecimalData a;
            a._data.rep = val;
            return a;
        }

        constexpr Rep to_bits() const
        {
            return _data.rep;
        }

    private:

        Data _data;
    };
}
