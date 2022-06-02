/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DecimalConstants.h"

#include <cstdint>
#include <cassert>
#include <limits>
#include <bit>

namespace awl
{
    template <typename UInt, uint8_t exp_len>
    class BuiltInDecimalData
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

        constexpr BuiltInDecimalData() : BuiltInDecimalData(true, 0, 0) {}
        
        constexpr BuiltInDecimalData(bool sign, uint8_t exp, UInt man) : m_pack{ sign ? 0u : 1u, exp, man } {}

        constexpr bool positive() const
        {
            return m_pack.sign == 0;
        }

        constexpr void set_positive(bool val)
        {
            m_pack.sign = val ? 0u : 1u;
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

        static constexpr BuiltInDecimalData from_bits(Rep val)
        {
            BuiltInDecimalData a;
            a.m_pack = std::bit_cast<Pack>(val);
            return a;
        }

        constexpr Rep to_bits() const
        {
            return std::bit_cast<Rep>(m_pack);
        }

    private:

        Pack m_pack;
    };
}
