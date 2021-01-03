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

    template <class T> struct remove_smart_pointer { using type = T; };
    template <class T> struct remove_smart_pointer<T*> { using type = T; };
    template <class T> struct remove_smart_pointer<T* const> { using type = T; };
    template <class T> struct remove_smart_pointer<T* volatile> { using type = T; };
    template <class T> struct remove_smart_pointer<T* const volatile> { using type = T; };
    template <class T> struct remove_smart_pointer<std::shared_ptr<T>> { using type = T; };
    template <class T, class Deleter> struct remove_smart_pointer<std::unique_ptr<T, Deleter>> { using type = T; };

    template <class T>
    using remove_smart_pointer_t = typename remove_smart_pointer<T>::type;
}
