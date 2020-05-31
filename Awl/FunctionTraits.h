#pragma once

#include <type_traits>
#include <tuple>

namespace awl
{
    // For generic types, directly use the result of the signature of its 'operator()'
    template <typename T>
    struct function_traits : public function_traits<decltype(&T::operator())> {};

    //We specialize for pointers to member function.
    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType(ClassType::*)(Args...) const>
    {
        enum { arity = sizeof...(Args) };
        // arity is the number of arguments.

        using result_type = ReturnType;

        template <size_t i>
        struct arg
        {
            using type = std::tuple_element_t<i, std::tuple<Args...>>;
            // the i-th argument is equivalent to the i-th tuple element of a tuple
            // composed of those arguments.
        };
    };
}
