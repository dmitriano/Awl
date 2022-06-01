/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DecimalConstants.h"

#include <cstdint>
#include <cassert>
#include <limits>
#include <array>
#include <bit>

#if defined(__GNUC__) || defined(__clang__)
#define AWL_DECIMAL_128 1
#endif

namespace awl
{
    template <typename UInt, uint8_t exp_len>
    class BuiltInDecimalData
    {
    public:

        using Int = std::make_signed_t<UInt>;

        constexpr BuiltInDecimalData() : BuiltInDecimalData(0, 0, 0) {}
        
        constexpr BuiltInDecimalData(UInt sign, UInt exp, UInt man) : m_pack{ sign, exp, man } {}

        constexpr bool positive() const
        {
            return m_pack.sign == 0;
        }

        constexpr void set_positive(bool val)
        {
            m_pack.sign = val ? 0 : 1;
        }

        constexpr uint8_t exp() const
        {
            return m_pack.exp;
        }

        constexpr void set_exp(UInt val)
        {
            m_pack.exp = val;
        }

        constexpr UInt man() const
        {
            return m_pack.man;
        }

        constexpr void set_man(UInt val)
        {
            m_pack.man = val;
        }

        static constexpr BuiltInDecimalData from_bits(UInt val)
        {
            BuiltInDecimalData a;
            a.m_pack = std::bit_cast<Pack>(val);
            return a;
        }

        constexpr UInt to_bits() const
        {
            return std::bit_cast<UInt>(m_pack);
        }

    private:

        using Constants = helpers::DecimalConstants<UInt, exp_len>;

        struct Pack
        {
            //0 - positive, 1 - negative
            UInt sign : Constants::sign_len;
            UInt exp : Constants::exp_len;
            UInt man : Constants::man_len;
        };

        Pack m_pack;
    };
}
