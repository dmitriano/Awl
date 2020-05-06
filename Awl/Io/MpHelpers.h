#pragma once

#include <tuple>
#include <variant>
#include <array>
#include <algorithm>
#include <type_traits>

namespace awl::io::helpers
{
    template <class V, class T, std::size_t... index>
    constexpr bool variant_contains_impl(std::index_sequence<index...>)
    {
        return (std::is_same_v<std::variant_alternative_t<index, V>, T> || ...);
    }

    template <class V, class T>
    constexpr inline bool variant_contains = variant_contains_impl<V, T>(std::make_index_sequence<std::variant_size_v<V>>());

    static_assert(variant_contains<std::variant<int, bool, double>, int>);
    static_assert(!variant_contains<std::variant<int, bool, double>, float>);
}