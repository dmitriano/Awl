/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Exception.h"
#include "Awl/TypeTraits.h"

#include <tuple>
#include <variant>
#include <array>
#include <algorithm>
#include <type_traits>

#include <utility>

namespace awl
{
//In VS2017 the value of __cplusplus is still 199711L even with enabled /std:c++17 option,
//but with C++ 17 it should be 201402L, so we cannot use it.

    template <typename ... Ts>
    constexpr std::size_t sizeof_tuple(std::tuple<Ts...> const &)
    {
        return (sizeof(Ts) + ...);
    }

    //Const for_each overloads for iterating over const std::tuple<Args & ...>.

    template <typename... Args, typename Func, std::size_t... index>
    constexpr void for_each(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        (f(std::get<index>(t)), ...);
    }

    template <typename... Args, typename Func>
    constexpr void for_each(const std::tuple<Args...>& t, Func&& f)
    {
        for_each(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    constexpr void for_each_index(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        (f(std::get<index>(t), std::integral_constant<std::size_t, index>()), ...);
    }

    template <typename... Args, typename Func>
    constexpr void for_each_index(const std::tuple<Args...>& t, Func&& f)
    {
        for_each_index(t, f, std::index_sequence_for<Args...>{});
    }

    //Non const for_each overloads for iterating over std::tuple<Args ...>.

    template <typename... Args, typename Func, std::size_t... index>
    constexpr void for_each(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        (f(std::get<index>(t)), ...);
    }

    template <typename... Args, typename Func>
    constexpr void for_each(std::tuple<Args...>& t, Func&& f)
    {
        for_each(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    constexpr void for_each_index(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        (f(std::get<index>(t), std::integral_constant<std::size_t, index>()), ...);
    }

    template <typename... Args, typename Func>
    constexpr void for_each_index(std::tuple<Args...>& t, Func&& f)
    {
        for_each_index(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    constexpr auto transform_tuple(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::make_tuple(f(std::get<index>(t)) ...);
    }

    template <typename... Args, typename Func>
    constexpr auto transform_tuple(const std::tuple<Args...>& t, Func&& f)
    {
        return transform_tuple(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    constexpr auto transform_tuple(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::make_tuple(f(std::get<index>(t)) ...);
    }

    template <typename... Args, typename Func>
    constexpr auto transform_tuple(std::tuple<Args...>& t, Func&& f)
    {
        return transform_tuple(t, f, std::index_sequence_for<Args...>{});
    }

    template <template <class> class T, class Tuple, std::size_t... index>
    constexpr auto transform_t2t(const Tuple & t, std::index_sequence<index...>)
    {
        return std::make_tuple(T<std::tuple_element_t<index, Tuple>>(std::get<index>(t)) ...);
    }

    template <template <class> class T, class Tuple>
    constexpr auto transform_t2t(const Tuple & t)
    {
        return transform_t2t<T>(t, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <template <size_t index> class T, class Tuple, std::size_t... index>
    constexpr auto transform_t2ti(const Tuple & t, std::index_sequence<index...>)
    {
        return std::make_tuple(T<index>(std::get<index>(t)) ...);
    }

    template <template <size_t index> class T, class Tuple>
    constexpr auto transform_t2ti(const Tuple & t)
    {
        return transform_t2ti<T>(t, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <class V, template <class> class T, std::size_t... index>
    constexpr auto transform_v2t(std::index_sequence<index...>)
    {
        return std::make_tuple(T<std::variant_alternative_t<index, V>>() ...);
    }

    template <class V, template <class> class T>
    constexpr auto transform_v2t()
    {
        return transform_v2t<V, T>(std::make_index_sequence<std::variant_size_v<V>>());
    }

    template <class V, template <size_t> class T, std::size_t... index>
    constexpr auto transform_v2ti(std::index_sequence<index...>)
    {
        return std::make_tuple(T<index>() ...);
    }

    template <class V, template <size_t> class T>
    constexpr auto transform_v2ti()
    {
        return transform_v2ti<V, T>(std::make_index_sequence<std::variant_size_v<V>>());
    }

    template <typename... Args, typename Func, std::size_t... index>
    constexpr auto tuple_to_array(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::array{f(std::get<index>(t)) ...};
    }

    template <typename... Args, typename Func>
    constexpr auto tuple_to_array(const std::tuple<Args...>& t, Func&& f)
    {
        return tuple_to_array(t, f, std::index_sequence_for<Args...>{});
    }

    template <class I, typename... Args, std::size_t... index>
    constexpr auto tuple_cast(const std::tuple<Args...>& t, std::index_sequence<index...>)
    {
        return std::array{ static_cast<I *>(&std::get<index>(t)) ... };
    }

    template <class I, typename... Args>
    constexpr auto tuple_cast(const std::tuple<Args...>& t)
    {
        return tuple_cast<I>(t, std::index_sequence_for<Args...>{});
    }

    template <class I, typename... Args, std::size_t... index>
    constexpr auto tuple_cast(std::tuple<Args...>& t, std::index_sequence<index...>)
    {
        return std::array{ static_cast<I *>(&std::get<index>(t)) ... };
    }

    template <class I, typename... Args>
    constexpr auto tuple_cast(std::tuple<Args...>& t)
    {
        return tuple_cast<I>(t, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    constexpr auto tuple_to_array(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::array{ f(std::get<index>(t)) ... };
    }

    template <typename... Args, typename Func>
    constexpr auto tuple_to_array(std::tuple<Args...>& t, Func&& f)
    {
        return tuple_to_array(t, f, std::index_sequence_for<Args...>{});
    }

    //Get the index of the single unique match for an arbitrary type in something tuple-like:
    template <class T, class U, std::size_t... index>
    static constexpr auto find_tuple_type_impl(std::index_sequence<index...>) noexcept
    {
        using NoRefT = std::remove_reference_t<T>;
        static_assert((std::size_t() + ... + std::is_same_v<NoRefT, std::tuple_element_t<index, U>>) == 1, "There is no single exact match");
        return std::max({ (std::is_same_v<NoRefT, std::tuple_element_t<index, U>> ? index : 0)... });
    }
    
    template <class T, class U>
    static constexpr std::size_t find_tuple_type_v = find_tuple_type_impl<T, U>(std::make_index_sequence<std::tuple_size_v<U>>());

    template <class T, class U, std::size_t... index>
    static constexpr auto find_variant_type_impl(std::index_sequence<index...>) noexcept
    {
        using NoRefT = std::remove_reference_t<T>;
        static_assert((std::size_t() + ... + std::is_same_v<NoRefT, std::variant_alternative_t<index, U>>) == 1, "There is no single exact match");
        return std::max({ (std::is_same_v<NoRefT, std::variant_alternative_t<index, U>> ? index : 0)... });
    }

    template <class T, class U>
    static constexpr std::size_t find_variant_type_v = find_variant_type_impl<T, U>(std::make_index_sequence<std::variant_size_v<U>>());

    //Use that to get all the indices and put them into a std::array:
    template <class U, class T, std::size_t... index>
    constexpr auto map_types_impl_t2t(std::index_sequence<index...>) noexcept
    {
        return std::array<std::size_t, sizeof...(index)>{find_tuple_type_v<std::tuple_element_t<index, U>, T>...};
    }
    
    //U is a tuple, T is a tuple
    template <class U, class T>
    constexpr auto map_types_t2t() noexcept
    {
        return map_types_impl_t2t<U, T>(std::make_index_sequence<std::tuple_size_v<U>>());
    }

    template <class U, class T, std::size_t... index>
    constexpr auto map_types_impl_t2v(std::index_sequence<index...>) noexcept
    {
        return std::array<std::size_t, sizeof...(index)>{find_variant_type_v<std::tuple_element_t<index, U>, T>...};
    }

    //U is a tuple, T is a variant
    template <class U, class T>
    constexpr auto map_types_t2v() noexcept
    {
        return map_types_impl_t2v<U, T>(std::make_index_sequence<std::tuple_size_v<U>>());
    }

    static_assert(sizeof_tuple(std::tuple<int, bool>{}) == sizeof(int) + sizeof(bool), "it is not 5!");

    // Makes a tuple consisting of both references and values.
    template <typename... Args>
    constexpr auto make_universal_tuple(Args&&... args)
    {
        // In the case you pass a std::string lvalue then T will deduce to std::string& or const std::string&, for rvalues it will deduce to std::string.
        return std::tuple<Args...>(args...);
    }

    // Casts its arguments to a given tuple element types preserving CV qualifiers and references and returns a tuple of that arguments.
    template <typename Tuple, typename... Args>
    constexpr auto make_similar_tuple(Args&&... args)
    {
        static_assert(std::tuple_size_v<Tuple> == sizeof...(Args));

        auto impl_func = [&]<size_t... Is>(std::index_sequence<Is...>)
        {
            return std::tuple<std::common_reference_t<std::tuple_element_t<Is, Tuple>&, Args>...>(std::forward<Args>(args)...);
        };

        return impl_func(std::index_sequence_for<Args...>{});
    }
}
