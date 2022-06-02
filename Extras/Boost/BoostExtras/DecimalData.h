/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DecimalConstants.h"
#include "Awl/Int2Array.h"

#include <boost/multiprecision/cpp_int.hpp>

#include <cstdint>
#include <cassert>
#include <limits>
#include <bit>

namespace awl
{
    template <typename UInt, uint8_t exp_len>
    class BoostDecimalData
    {
    private:

        using Constants = helpers::DecimalConstants<UInt, exp_len>;

    public:

        //using Rep = std::array<std::uint8_t, sizeof(Pack)>;

        using Int = std::make_signed_t<UInt>;

        /*
        constexpr BoostDecimalData() : BoostDecimalData(0, 0, 0) {}
        
        constexpr BoostDecimalData(UInt sign, UInt exp, UInt man) : m_pack{ sign, exp, man } {}

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

        static constexpr BoostDecimalData from_bits(Rep val)
        {
            BoostDecimalData a;
            a.m_pack = std::bit_cast<Pack>(val);
            return a;
        }

        constexpr Rep to_bits() const
        {
            return std::bit_cast<Rep>(m_pack);
        }

    private:

        Pack m_pack;
        */
    };
}
