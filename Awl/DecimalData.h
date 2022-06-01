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
        
        constexpr BuiltInDecimalData(UInt sign, UInt exp, UInt man) :
            m_sign(sign), m_exp(exp), m_man(man)
        {
        }

        constexpr UInt sign() const
        {
            return m_sign;
        }

        constexpr uint8_t exp() const
        {
            return m_exp;
        }

        constexpr UInt man() const
        {
            return m_man;
        }

        constexpr void set_sign(UInt val)
        {
            m_sign = val;
        }

        constexpr void set_exp(UInt val)
        {
            m_exp = val;
        }

        constexpr void set_man(UInt val)
        {
            m_man = val;
        }

    private:

        using Constants = helpers::DecimalConstants<UInt, exp_len>;

        //0 - positive, 1 - negative
        UInt m_sign : Constants::sign_len;
        UInt m_exp : Constants::exp_len;
        UInt m_man : Constants::man_len;
    };
}
