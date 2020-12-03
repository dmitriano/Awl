#pragma once

#include <type_traits>

namespace awl
{
    template<typename Test, template<typename...> class Ref>
    struct is_specialization : std::false_type {};

    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

    template<typename Test, template<typename...> class Ref>
    constexpr bool is_specialization_v = is_specialization<Test, Ref>::value;

    
    template<class T> struct dependent_false : std::false_type {};

    template <typename T>
    inline constexpr bool dependent_false_v = dependent_false<T>::value;
}
