/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>
#include <ranges>

namespace awl
{
    template <class T> requires std::is_integral_v<T>
    auto make_int_range(T begin, T end)
    {
        return std::views::iota(begin, end);
    }

    template <class T> requires std::is_integral_v<T>
    auto make_count(T end)
    {
        return make_int_range(static_cast<T>(0), end);
    }
}
