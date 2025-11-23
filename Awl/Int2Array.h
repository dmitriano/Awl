/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <array>
#include <type_traits>
#include <bit>

namespace awl
{
    template <typename T>
    constexpr std::array<std::uint8_t, sizeof(T)> to_array(T value)
    {
        return std::bit_cast<std::array<std::uint8_t, sizeof(T)>>(value);
    }

    template <typename T>
    constexpr T from_array(const std::array<std::uint8_t, sizeof(T)>& a)
    {
        return std::bit_cast<T>(a);
    }
}
