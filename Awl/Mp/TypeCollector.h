/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Tuplizable.h"
#include "Awl/TupleHelpers.h"

#include <tuple>
#include <type_traits>
#include <string>
#include <vector>
#include <array>
#include <ranges>

namespace awl::mp
{
    template <typename T>
    struct type_collector
    {
        using Tuple = std::tuple<T>;
    };

    template <std::ranges::range Coll>
    struct type_collector<Coll>
    {
        using Tuple = awl::tuple_cat_t<
            std::tuple<Coll>,
            typename type_collector<std::ranges::range_value_t<Coll>>::Tuple>;
    };

    template <class First, class Second>
    struct type_collector<std::pair<First, Second>>
    {
        using Tuple = awl::tuple_cat_t<
            typename type_collector<std::decay_t<First>>::Tuple,
            typename type_collector<std::decay_t<Second>>::Tuple>;
    };

    // Remove references and CV from tuple elments.
    template <typename ... Ts>
    struct type_collector<std::tuple<Ts& ...>>
    {
        using Tuple = awl::tuple_cat_t<
            typename type_collector<std::decay_t<Ts>>::Tuple ...>;
    };

    template <typename T> requires is_tuplizable_v<T>
    struct type_collector<T>
    {
        using Tuple = awl::tuple_cat_t <
            std::tuple<T>,
            typename type_collector<typename tuplizable_traits<T>::Tie>::Tuple>;
    };

    // Make std::string a final type, do not add char.
    template<class Char, class Traits, class Allocator> requires std::is_arithmetic_v<Char>
    struct type_collector<std::basic_string<Char, Traits, Allocator>>
    {
        using Tuple = std::tuple<std::basic_string<Char, Traits, Allocator>>;
    };

    // Make arithmetic vector a final type.
    template<typename T,class Allocator> requires std::is_arithmetic_v<T>
    struct type_collector<std::vector<T, Allocator>>
    {
        using Tuple = std::tuple<std::vector<T, Allocator>>;
    };

    // Make arithmetic array a final type.
    template<typename T, std::size_t N> requires std::is_arithmetic_v<T>
    struct type_collector<std::array<T, N>>
    {
        using Tuple = std::tuple<std::array<T, N>>;
    };
}
