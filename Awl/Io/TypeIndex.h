#pragma once

#include <type_traits>
#include <limits>

namespace awl::io
{
    template <class T>
    constexpr std::size_t countof_fields();

    template <class Tuple, std::size_t... index>
    constexpr std::size_t countof_tuple_fields(std::index_sequence<index...>)
    {
        return (countof_fields<std::remove_reference_t<std::tuple_element_t<index, Tuple>>>() + ...);
    }

    template <class T>
    constexpr std::size_t countof_fields()
    {
        if constexpr (is_tuplizable_v<T>)
        {
            using Tie = typename tuplizable_traits<T>::Tie;
            return countof_tuple_fields<Tie>(std::make_index_sequence<std::tuple_size_v<Tie>>());
        }
        else
        {
            return 1;
        }
    }

    inline constexpr size_t no_type_index = std::numeric_limits<size_t>::max();

    template <class Struct, class Field>
    constexpr size_t indexof_type();

    template <class Tuple, class Field, std::size_t... index>
    constexpr size_t indexof_tuple_type(std::index_sequence<index...>)
    {
        constexpr size_t element_index = std::min({ (indexof_type<std::remove_reference_t<std::tuple_element_t<index, Tuple>>, Field>() != no_type_index ?
            index : no_type_index)... });

        if constexpr (element_index != no_type_index)
        {
            using Struct = std::remove_reference_t<std::tuple_element_t<element_index, Tuple>>;

            const size_t local_index = indexof_type<Struct, Field>();

            if constexpr (element_index != 0)
            {
                constexpr size_t predecessor_count = countof_tuple_fields<Tuple>(std::make_index_sequence<element_index>());

                return predecessor_count + local_index;
            }
            else
            {
                //A special case for the empty index_sequence to prevent compiler error
                //'a unary fold expression over '+' must have a non-empty expansion'.
                return local_index;
            }
        }
        else
        {
            return no_type_index;
        }
    }

    template <class Struct, class Field>
    constexpr size_t indexof_type()
    {
        if constexpr (is_tuplizable_v<Struct>)
        {
            using Tie = typename tuplizable_traits<Struct>::Tie;
            return indexof_tuple_type<Tie, Field>(std::make_index_sequence<std::tuple_size_v<Tie>>());
        }
        else
        {
            return std::is_same_v<Field, Struct> ? 0 : no_type_index;
        }
    }
}
