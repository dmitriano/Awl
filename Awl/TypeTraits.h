/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>
#include <memory>

namespace awl
{
    //is_specialization implementation

    template<typename Test, template<typename...> class Ref>
    struct is_specialization : std::false_type {};

    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

    template<typename Test, template<typename...> class Ref>
    constexpr bool is_specialization_v = is_specialization<Test, Ref>::value;

    //dependent_false implementation
    
    template<class T> struct dependent_false : std::false_type {};

    template <typename T>
    inline constexpr bool dependent_false_v = dependent_false<T>::value;

    //remove_pointer implementation
    
    template <class T> struct remove_pointer { using type = T; };
    template <class T> struct remove_pointer<T*> { using type = T; };
    template <class T> struct remove_pointer<T* const> { using type = T; };
    template <class T> struct remove_pointer<T* volatile> { using type = T; };
    template <class T> struct remove_pointer<T* const volatile> { using type = T; };
    template <class T> struct remove_pointer<std::shared_ptr<T>> { using type = T; };
    template <class T, class Deleter> struct remove_pointer<std::unique_ptr<T, Deleter>> { using type = T; };

    template <class T>
    using remove_pointer_t = typename remove_pointer<T>::type;

    //is_plain_pointer implementation

    template<class T>
    struct is_plain_pointer : std::false_type {};

    template<class T>
    struct is_plain_pointer<T *> : std::true_type {};

    template<class T>
    constexpr bool is_plain_pointer_v = is_plain_pointer<T>::value;

    //Pointer related helpers.

    template<class T>
    constexpr bool is_smart_pointer_v = is_specialization_v<T, std::shared_ptr> || is_specialization_v<T, std::unique_ptr>;

    template<class T>
    constexpr bool is_pointer_v = is_plain_pointer_v<T> || is_smart_pointer_v<T>;

    template<class T>
    constexpr bool is_copyable_pointer_v = is_plain_pointer_v<T> || is_specialization_v<T, std::shared_ptr>;

    template<class T>
    auto * object_address(T& val)
    {
        //it can be 'const std::shared_ptr<A>', so remove 'const'.
        using dT = std::decay_t<T>;
        
        if constexpr (is_plain_pointer_v<dT>)
        {
            return val;
        }
        else if constexpr (is_smart_pointer_v<dT>)
        {
            return val.get();
        }
        else
        {
            return &val;
        }
    }

    //We can't return both std::shared_ptr and A&.
    //template<class T>
    //auto object_reference(T& val)
    //{
    //    if constexpr (is_pointer_v<T>)
    //    {
    //        return *val;
    //    }
    //    else
    //    {
    //        return val;
    //    }
    //}

    template<typename T, typename = void>
    constexpr bool is_defined_v = false;

    template<typename T>
    constexpr bool is_defined_v<T, decltype(typeid(T), void())> = true;
}
