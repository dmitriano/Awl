/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <array>
#include <type_traits>

namespace awl
{
    template <typename T>
    constexpr std::array<std::uint8_t, sizeof(T)> to_array(T value)
    {
        //'= {}' is for preventing GCC warning "there is no default constructor..."
        std::array<std::uint8_t, sizeof(T)> result = {};

        for (std::size_t i{ sizeof(T) }; i != 0; --i)
        {
            result[i - 1] = static_cast<uint8_t>(value >> ((sizeof(T) - i) * 8));
        }

        return result;
    }

    template <typename T>
    constexpr T from_array(const std::array<std::uint8_t, sizeof(T)>& a)
    {
        T val = 0;

        for (uint8_t byte : a)
        {
            val = (val << 8) + byte;
        }

        return val;
    }
}
