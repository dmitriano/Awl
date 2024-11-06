/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Tuplizable.h"

#include <tuple>
#include <variant>
#include <array>
#include <algorithm>
#include <type_traits>
#include <ranges>

namespace awl::mp
{
    // A quick solution for extracting tuplizable types from a container.
    template <class Coll>
    constexpr auto extract_element_types()
    {
        if constexpr (std::ranges::range<Coll>)
        {
            using T = std::ranges::range_value_t<Coll>;

            if constexpr (awl::is_specialization_v<T, std::pair>)
            {
                return std::tuple_cat(
                    extract_element_types<std::decay_t<typename T::first_type>>(),
                    extract_element_types<std::decay_t<typename T::second_type>>());
            }
            else
            {
                return extract_element_types<T>();
            }
        }
        else
        {
            if constexpr (is_tuplizable_v<Coll>)
            {
                return std::tuple<Coll>{};
            }
            else
            {
                return std::tuple<>{};
            }
        }
    }
}
