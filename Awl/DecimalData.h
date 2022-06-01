/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <cassert>
#include <limits>
#include <array>
#include <cmath>
#include <bit>

#if defined(__GNUC__) || defined(__clang__)
#define AWL_DECIMAL_128 1
#endif

namespace awl
{
    namespace helpers
    {
        template <typename UInt, uint8_t exp_length>
        struct DecimalConstants
        {
            static constexpr uint8_t sign_len = 1;
            //With UInt and exp_len == 4 the mantissa length is 59.
            static constexpr uint8_t exp_len = exp_length;
            static constexpr uint8_t man_len = sizeof(UInt) * 8 - (exp_len + sign_len);

            static constexpr UInt p2(uint8_t n)
            {
                return static_cast<UInt>(1) << n;
            }

            static constexpr UInt max_exp()
            {
                return p2(exp_len) - 1;
            }

            static constexpr UInt max_man()
            {
                return p2(man_len) - 1;
            }

            static constexpr uint8_t log10(uint8_t len)
            {
                UInt val = p2(len) - 1;

                uint8_t count = 0;

                while ((val /= 10) != 0)
                {
                    ++count;
                }

                return count;
            }

            using DenomArray = std::array<UInt, p2(exp_len)>;

            static constexpr DenomArray make_denoms()
            {
                DenomArray a{};

                UInt denom = 1;

                for (UInt e = 0; e < p2(exp_len); ++e)
                {
                    a[e] = denom;

                    denom *= 10;
                }

                return a;
            }
        };

        static_assert(DecimalConstants<uint64_t, 4>::man_len == 59u);
    }
    
    template <typename UInt, uint8_t exp_len>
    class BuiltInDecimalData
    {
    public:

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
