/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Mp/TupleToVariant.h"
#include "Awl/Mp/ExtractElementTypes.h"

#include "Awl/Tuplizable.h"

#include <tuple>
#include <variant>
#include <array>
#include <algorithm>
#include <type_traits>
#include <ranges>

namespace awl::mp
{
    //GV construction

    //We pass tuple of references by value.
    template <class... Ts>
    constexpr auto flatten_tuple(std::tuple<Ts...> t);

    template <class T>
    constexpr auto flatten_object(T & val)
    {
        auto val_tuple = std::make_tuple(static_cast<T>(val));

        if constexpr (is_tuplizable_v<T>)
        {
            return std::tuple_cat(val_tuple, flatten_tuple(object_as_tuple(val)));
        }
        else
        {
            return std::tuple_cat(val_tuple, extract_element_types<T>());
        }
    }

    template <class... Ts, std::size_t... index>
    constexpr auto flatten_tuple_impl(std::tuple<Ts...> t, std::index_sequence<index...>)
    {
        return std::tuple_cat(flatten_object(std::get<index>(t))...);
    }

    template <class... Ts>
    constexpr auto flatten_tuple(std::tuple<Ts...> t)
    {
        return flatten_tuple_impl(t, std::index_sequence_for<Ts...>{});
    }

    template <class T>
    constexpr auto flatten_struct()
    {
        T val = {};
        return flatten_object(val);
    }
}
