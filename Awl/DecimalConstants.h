/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <cassert>
#include <limits>
#include <cmath>
#include <array>

namespace awl
{
    namespace helpers
    {
        template <typename UInt, uint8_t exp_length, uint8_t type_size = sizeof(UInt)>
        struct DecimalConstants
        {
            static constexpr uint8_t sign_len = 1;
            //With UInt and exp_len == 4 the mantissa length is 59.
            static constexpr uint8_t exp_len = exp_length;
            //TODO: uint8_t is not enough for boost::multiprecision
            static constexpr uint8_t man_len = type_size * 8 - (exp_len + sign_len);

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

            using DenomArray = std::array<UInt, static_cast<size_t>(p2(exp_len))>;

            static constexpr DenomArray make_denoms()
            {
                DenomArray a{};

                UInt denom = 1;

                for (size_t e = 0; e < static_cast<size_t>(p2(exp_len)); ++e)
                {
                    a[e] = denom;

                    denom *= 10;
                }

                return a;
            }
        };

        static_assert(DecimalConstants<uint64_t, 4>::man_len == 59u);
    }
}
