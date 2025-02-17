/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>
#include <memory>
#include <concepts>
#include <utility>
#include <string>
#include <ranges>

#ifdef AWL_QT
#include <QString>
#endif //AWL_QT

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

    template <class T>
    concept is_string = is_specialization_v<T, std::basic_string>
#ifdef AWL_QT
        || std::is_same_v<T, QString>
#endif
    ;

    // Standard container concepts.

    template <class Container>
    concept insertable_map = std::ranges::range<Container> &&
        requires(Container& container)
    {
        typename Container::key_type;
        typename Container::mapped_type;
        std::is_same_v<std::pair<const typename Container::key_type, typename Container::mapped_type>, std::ranges::range_value_t<Container>>;
        { container.insert(std::declval<std::ranges::range_value_t<Container>&&>()) };
    };

    template <class Container>
    concept insertable_sequence = std::ranges::range<Container> && !insertable_map<Container> &&
        requires(Container& container)
    {
        { container.insert(std::declval<std::ranges::range_value_t<Container>&&>()) };
    };

    // std::basic_string and QString have push_back method.
    template <class Container>
    concept back_insertable_sequence = std::ranges::range<Container> && !is_string<Container> &&
        requires(Container& container)
    {
        { container.push_back(std::declval<std::ranges::range_value_t<Container>&&>()) };
    };
}
