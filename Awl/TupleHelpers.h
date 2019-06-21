#pragma once

#include <tuple>
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

    template <class T1, class T2>
    inline constexpr auto map_types(const T1 & t1, T2 & t2)
    {
        std::array<size_t, std::tuple_size<T2>::value> a;
        a.fill(static_cast<size_t>(-1));

        for_each_index(t2, [&a, &t1](const auto & field2, size_t index2)
        {
            for_each_index(t1, [&a, &field2, &index2](const auto & field1, size_t index1)
            {
                if constexpr (std::is_same_v<decltype(field1), decltype(field2)>)
                {
                    a[index2] = index1;
                }
                else
                {
                    static_cast<void>(index1);
                }
            });
        });
        
        return a;
    }

    template <class T1, class T2>
    inline constexpr auto map_types()
    {
        const T1 t1{};
        const T2 t2{};

        return map_types(t1, t2);
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
