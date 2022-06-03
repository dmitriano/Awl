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
        struct multiprecision_descriptor;

        template <>
        struct multiprecision_descriptor<boost::multiprecision::uint128_t>
        {
            using signed_type = boost::multiprecision::int128_t;

            static constexpr std::size_t size = 16;
        };

        template <>
        struct multiprecision_descriptor<boost::multiprecision::uint256_t>
        {
            using signed_type = boost::multiprecision::int256_t;

            static constexpr std::size_t size = 32;
        };

        template <>
        struct multiprecision_descriptor<boost::multiprecision::uint512_t>
        {
            using signed_type = boost::multiprecision::int512_t;

            static constexpr std::size_t size = 64;
        };

        template <>
        struct multiprecision_descriptor<boost::multiprecision::uint1024_t>
        {
            using signed_type = boost::multiprecision::int1024_t;

            static constexpr std::size_t size = 128;
        };

        template <class T>
        using make_signed_t = typename multiprecision_descriptor<T>::signed_type;
    }

    template <typename UInt, uint8_t exp_len>
    class MultiprecisionDecimalData
    {
    private:

        static constexpr size_t type_size = detail::multiprecision_descriptor<UInt>::size;
        
        using Constants = helpers::DecimalConstants<UInt, exp_len, type_size>;

    public:

        using Rep = UInt;

        using Int = detail::make_signed_t<UInt>;

        constexpr MultiprecisionDecimalData() : MultiprecisionDecimalData(true, 0, 0) {}
        
        constexpr MultiprecisionDecimalData(bool pos, uint8_t exp, UInt man)
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

        static constexpr MultiprecisionDecimalData from_bits(Rep val)
        {
            MultiprecisionDecimalData a;
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
