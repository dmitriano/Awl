/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DecimalConstants.h"

#include <boost/multiprecision/cpp_int.hpp>

#include <cstdint>
#include <cassert>
#include <limits>
#include <bit>

namespace awl
{
    namespace detail
    {
        template <class T>
        struct make_signed;

        template <>
        struct make_signed<boost::multiprecision::uint128_t>
        {
            using type = boost::multiprecision::int128_t;
        };

        template <>
        struct make_signed<boost::multiprecision::uint256_t>
        {
            using type = boost::multiprecision::int256_t;
        };

        template <>
        struct make_signed<boost::multiprecision::uint512_t>
        {
            using type = boost::multiprecision::int512_t;
        };

        template <>
        struct make_signed<boost::multiprecision::uint1024_t>
        {
            using type = boost::multiprecision::int1024_t;
        };

        template <class T>
        using make_signed_t = typename make_signed<T>::type;
    }

    template <typename UInt, uint8_t exp_len>
    class BoostDecimalData
    {
    private:

        static constexpr size_t type_size = sizeof(UInt) - 8;
        
        using Constants = helpers::DecimalConstants<UInt, exp_len, type_size>;

    public:

        using Rep = UInt;

        using Int = detail::make_signed_t<UInt>;

        constexpr BoostDecimalData() : BoostDecimalData(true, 0, 0) {}
        
        constexpr BoostDecimalData(bool pos, uint8_t exp, UInt man)
        {
            set_positive(pos);
            set_exp(exp);
            set_man(man);
        }

        constexpr bool positive() const
        {
            return (m_val & signMask) == 0;
        }

        constexpr void set_positive(bool pos)
        {
            m_val = (m_val & ~signMask) | (pos ? 0x0 : 0x1);
        }

        constexpr uint8_t exp() const
        {
            return static_cast<uint8_t>((m_val & expMask) >> 1);
        }

        constexpr void set_exp(uint8_t val)
        {
            m_val = m_val & ~expMask | (UInt(val) << 1);
        }

        constexpr UInt man() const
        {
            return m_val >> (Constants::exp_len + 1);
        }

        constexpr void set_man(UInt val)
        {
            m_val = m_val & ~manMask | (UInt(val) << (Constants::exp_len + 1));
        }

        static constexpr BoostDecimalData from_bits(Rep val)
        {
            BoostDecimalData a;
            a.m_val = val;
            return a;
        }

        constexpr Rep to_bits() const
        {
            return m_val;
        }

    private:

        static constexpr uint8_t signMask = 0x1;

        static constexpr UInt expMask = ((UInt(1) << Constants::exp_len) - 1) << 1;

        static constexpr UInt manMask = UInt(-1) & ~(expMask | signMask);

        UInt m_val;
    };
}
