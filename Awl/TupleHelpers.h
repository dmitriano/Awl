#pragma once

#include <tuple>

#if AWL_CPPSTD >= 17
#include <utility>
#endif

namespace awl
{
//In VS2017 the value of __cplusplus is still 199711L even with enabled /std:c++17 option,
//but with C++ 17 it should be 201402L, so we cannot use it.
#if AWL_CPPSTD >= 17

    //In C++17 template folding can be used:
    template <typename ... Ts>
    constexpr std::size_t sizeof_tuple(std::tuple<Ts...> const &)
    {
        return (sizeof(Ts) + ...);
    }

    template <typename... Args, typename Func, std::size_t... Idx>
    inline constexpr void for_each(const std::tuple<Args...>& t, Func&& f, std::index_sequence<Idx...>)
    {
        (f(std::get<Idx>(t)), ...);
    }

    template <typename... Args, typename Func>
    inline constexpr void for_each(const std::tuple<Args...>& t, Func&& f)
    {
        for_each(t, f, std::index_sequence_for<Args...>{});
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
