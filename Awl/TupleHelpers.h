#pragma once

#include "Awl/Exception.h"

#include <tuple>
#include <variant>
#include <array>
#include <algorithm>

#if AWL_CPPSTD >= 17
#include <utility>
#endif

namespace awl
{
//In VS2017 the value of __cplusplus is still 199711L even with enabled /std:c++17 option,
//but with C++ 17 it should be 201402L, so we cannot use it.
#if AWL_CPPSTD >= 17

    template <typename ... Ts>
    constexpr std::size_t sizeof_tuple(std::tuple<Ts...> const &)
    {
        return (sizeof(Ts) + ...);
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr void for_each(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        (f(std::get<index>(t)), ...);
    }

    template <typename... Args, typename Func>
    inline constexpr void for_each(const std::tuple<Args...>& t, Func&& f)
    {
        for_each(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr void for_each_index(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        (f(std::get<index>(t), index), ...);
    }

    template <typename... Args, typename Func>
    inline constexpr void for_each_index(const std::tuple<Args...>& t, Func&& f)
    {
        for_each_index(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr auto transform_tuple(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::make_tuple(f(std::get<index>(t)) ...);
    }

    template <typename... Args, typename Func>
    inline constexpr auto transform_tuple(const std::tuple<Args...>& t, Func&& f)
    {
        return transform_tuple(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr auto transform_tuple(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::make_tuple(f(std::get<index>(t)) ...);
    }

    template <typename... Args, typename Func>
    inline constexpr auto transform_tuple(std::tuple<Args...>& t, Func&& f)
    {
        return transform_tuple(t, f, std::index_sequence_for<Args...>{});
    }

    template <template <class> class T, class Tuple, std::size_t... index>
    inline constexpr auto transform_t2t(const Tuple & t, std::index_sequence<index...>)
    {
        return std::make_tuple(T<std::tuple_element_t<index, Tuple>>(std::get<index>(t)) ...);
    }

    template <template <class> class T, class Tuple>
    inline constexpr auto transform_t2t(const Tuple & t)
    {
        return transform_t2t<T>(t, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <template <size_t index> class T, class Tuple, std::size_t... index>
    inline constexpr auto transform_t2ti(const Tuple & t, std::index_sequence<index...>)
    {
        return std::make_tuple(T<index>(std::get<index>(t)) ...);
    }

    template <template <size_t index> class T, class Tuple>
    inline constexpr auto transform_t2ti(const Tuple & t)
    {
        return transform_t2ti<T>(t, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <class V, template <class> class T, std::size_t... index>
    inline constexpr auto transform_v2t(std::index_sequence<index...>)
    {
        return std::make_tuple(T<std::variant_alternative_t<index, V>>() ...);
    }

    template <class V, template <class> class T>
    inline constexpr auto transform_v2t()
    {
        return transform_v2t<V, T>(std::make_index_sequence<std::variant_size_v<V>>());
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr auto tuple_to_array(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::array{f(std::get<index>(t)) ...};
    }

    template <typename... Args, typename Func>
    inline constexpr auto tuple_to_array(const std::tuple<Args...>& t, Func&& f)
    {
        return tuple_to_array(t, f, std::index_sequence_for<Args...>{});
    }

    template <class I, typename... Args, std::size_t... index>
    inline constexpr auto tuple_cast(const std::tuple<Args...>& t, std::index_sequence<index...>)
    {
        return std::array{ static_cast<I *>(&std::get<index>(t)) ... };
    }

    template <class I, typename... Args>
    inline constexpr auto tuple_cast(const std::tuple<Args...>& t)
    {
        return tuple_cast<I>(t, std::index_sequence_for<Args...>{});
    }

    template <class I, typename... Args, std::size_t... index>
    inline constexpr auto tuple_cast(std::tuple<Args...>& t, std::index_sequence<index...>)
    {
        return std::array{ static_cast<I *>(&std::get<index>(t)) ... };
    }

    template <class I, typename... Args>
    inline constexpr auto tuple_cast(std::tuple<Args...>& t)
    {
        return tuple_cast<I>(t, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr auto tuple_to_array(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::array{ f(std::get<index>(t)) ... };
    }

    template <typename... Args, typename Func>
    inline constexpr auto tuple_to_array(std::tuple<Args...>& t, Func&& f)
    {
        return tuple_to_array(t, f, std::index_sequence_for<Args...>{});
    }

    template<class T> struct dependent_false : std::false_type {};

    template <typename T>
    inline constexpr bool dependent_false_v = dependent_false<T>::value;

    //Get the index of the single unique match for an arbitrary type in something tuple-like:
    template <class T, class U, std::size_t... index>
    static constexpr auto find_tuple_type_impl(std::index_sequence<index...>) noexcept
    {
        typedef std::remove_reference_t<T> NoRefT;
        static_assert((std::size_t() + ... + std::is_same_v<NoRefT, std::tuple_element_t<index, U>>) == 1, "There is no single exact match");
        return std::max({ (std::is_same_v<NoRefT, std::tuple_element_t<index, U>> ? index : 0)... });
    }
    
    template <class T, class U>
    static constexpr std::size_t find_tuple_type_v = find_tuple_type_impl<T, U>(std::make_index_sequence<std::tuple_size_v<U>>());

    template <class T, class U, std::size_t... index>
    static constexpr auto find_variant_type_impl(std::index_sequence<index...>) noexcept
    {
        typedef std::remove_reference_t<T> NoRefT;
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

    template <class Variant, class Tuple, std::size_t Index = 0>
    Variant runtime_get(Tuple &&tuple, std::size_t index)
    {
        if constexpr (Index == std::tuple_size_v<std::decay_t<Tuple>>)
        {
            static_cast<void>(tuple);
            static_cast<void>(index);
            throw GeneralException(_T("Index out of range for tuple"));
        }
        else
        {
            if (index == Index)
            {
                return Variant{ std::get<Index>(tuple) };
            }
            
            return runtime_get<Variant, Tuple, Index + 1>(std::forward<Tuple>(tuple), index);
        }
    }

    template <class Tuple, class Variant, std::size_t Index = 0>
    void runtime_set(Tuple &tuple, std::size_t index, Variant const& variant)
    {
        if constexpr (Index == std::tuple_size_v<std::decay_t<Tuple>>)
        {
            static_cast<void>(tuple);
            static_cast<void>(index);
            static_cast<void>(variant);
            throw GeneralException(_T("Index out of range for tuple"));
        }
        else
        {
            if (index == Index)
            {
                // Note: You should check here that variant holds the correct type
                // before assigning.
                std::get<Index>(tuple) = std::get<std::remove_reference_t<std::tuple_element_t<Index, Tuple>>>(variant);
            }
            else
            {
                runtime_set<Tuple, Variant, Index + 1>(tuple, index, variant);
            }
        }
    }

#elif AWL_CPPSTD >= 14

    //C++14 version (does not compile with GCC 4.9):
    template <typename ... Ts>
    constexpr std::size_t sizeof_tuple(std::tuple<Ts...> const &)
    {
        using unused = std::size_t[];

        std::size_t  sum{};

        //A variadic pack is expanded in an initialization list of a C-style array,
        //so it's initialized an used array and, inside the initialization list,
        //it's placed ret += sizeof(Ts) that is expanded for every type of the tuple. 
        //So ret accumulate the sum of the types.
        static_cast<void>(unused {0u, sum += sizeof(Ts)...});

        return sum;
    }

#else

    //C++11 version (compiles with GCC 4.9):
    template <typename = void>
    constexpr std::size_t sizeof_tuple_helper()
    {
        return 0u;
    }

    template <std::size_t I0, std::size_t ... Is>
    constexpr std::size_t sizeof_tuple_helper()
    {
        return I0 + sizeof_tuple_helper<Is...>();
    }

    template <typename ... Ts>
    constexpr std::size_t sizeof_tuple(std::tuple<Ts...> const &)
    {
        return sizeof_tuple_helper<sizeof(Ts)...>();
    }

#endif
    
    static_assert(sizeof_tuple(std::tuple<int, bool>{}) == sizeof(int) + sizeof(bool), "it is not 5!");
}
