#pragma once

#include <type_traits>

namespace awl
{
#if AWL_CPPSTD >= 17

    // For generic types, directly use the result of the signature of its 'operator()'
    template <typename T>
    struct function_traits : public function_traits<decltype(&T::operator())> {};

    //We specialize for pointers to member function.
    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType(ClassType::*)(Args...) const>
    {
        enum { arity = sizeof...(Args) };
        // arity is the number of arguments.

        typedef ReturnType result_type;

        template <size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
            // the i-th argument is equivalent to the i-th tuple element of a tuple
            // composed of those arguments.
        };
    };

#else

    template <typename T>
    struct return_type;
    template <typename R, typename... Args>
    struct return_type<R(*)(Args...)> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...)> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...) const> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...) volatile> { using type = R; };
    template <typename R, typename C, typename... Args>
    struct return_type<R(C::*)(Args...) const volatile> { using type = R; };
    template <typename T>
    using return_type_t = typename return_type<T>::type;

#endif
}
