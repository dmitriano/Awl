#pragma once

#include <tuple>
#include <variant>
#include <array>

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
    inline constexpr auto transform(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::make_tuple(f(std::get<index>(t)) ...);
    }

    template <typename... Args, typename Func>
    inline constexpr auto transform(const std::tuple<Args...>& t, Func&& f)
    {
        return transform(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr auto transform(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::make_tuple(f(std::get<index>(t)) ...);
    }

    template <typename... Args, typename Func>
    inline constexpr auto transform(std::tuple<Args...>& t, Func&& f)
    {
        return transform(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr auto to_array(const std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::array{f(std::get<index>(t)) ...};
    }

    template <typename... Args, typename Func>
    inline constexpr auto to_array(const std::tuple<Args...>& t, Func&& f)
    {
        return to_array(t, f, std::index_sequence_for<Args...>{});
    }

    template <typename... Args, typename Func, std::size_t... index>
    inline constexpr auto to_array(std::tuple<Args...>& t, Func&& f, std::index_sequence<index...>)
    {
        return std::array{ f(std::get<index>(t)) ... };
    }

    template <typename... Args, typename Func>
    inline constexpr auto to_array(std::tuple<Args...>& t, Func&& f)
    {
        return to_array(t, f, std::index_sequence_for<Args...>{});
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
